#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>

#include "jumper.h"
#include "job.h"

#define SOLID_SQUARE 'T'

#define GROUND_HEIGHT 5

static struct termios orig_state;
static vec screen;
static int nrows, ncols;

static void cleanup() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_state);
}

#include <assert.h>
static void setup() {
  assert(getpid() == tcgetpgrp(STDIN_FILENO));
  
  struct termios ttystate;
  if (tcgetattr(STDIN_FILENO, &ttystate) < 0) {
    printf("tcgetattor error: %s\n", strerror(errno));
    exit(0xbad);
  }
  orig_state = ttystate;

  ttystate.c_lflag &= ~(ICANON | ECHO);
  // set read to always return immediately
  ttystate.c_cc[VMIN] = 0;
  ttystate.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &ttystate) < 0) {
    printf("tcsetattr error: %s\n", strerror(errno));
    exit(0xbad);
  }

  screen = vec_new(sizeof(char), 0, NULL);
}

static bool kbhit() {
  struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
  if (poll(&pfd, 1, 1) <= 0) return false;
  return pfd.revents & POLLIN;
}

static int get_key() {
  return kbhit() ? getchar() : 0;
}

static inline void set_screen(int r, int c, char val) {
  *(char*)vec_get(&screen, r * ncols + c) = val;
}

static inline char get_screen(int r, int c) {
  return *(char*)vec_get(&screen, r * ncols + c);
}

// TODO: Figure out why the numbers are so big when using emacs
static void clear_screen() {
  struct winsize wsz;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz) < 0) {
    printf("ioctl error: %s", strerror(errno));
    exit(0xbad);
  }

  nrows = wsz.ws_row;
  ncols = wsz.ws_col;

  vec_resize(&screen, nrows * ncols + 1);
  memset(screen.data, ' ', nrows * ncols);
  *(char*)vec_back(&screen) = '\0';
}

static void add_background() {
  for (int r = nrows - 1; r >= nrows - GROUND_HEIGHT; --r) {
    for (int c = 0; c < ncols; c++) {
      set_screen(r, c, SOLID_SQUARE);
    }
  }
}

static void display_screen() {
  char* screen_str = screen.data;
  printf("%s\n", screen_str);
  /*
  for (int r = 0; r < nrows; r++) {
    for (int c = 0; c < ncols; c++) {
      printf("%c", get_screen(r, c));
    }
  }
  */
}

static void jumper() {
  setup();
  atexit(cleanup);
  clear_screen();
  for (int key = 0; key != 'q' && key != EOF; key = get_key()) {
    usleep(500);

    if (key != 0) {
      printf("Screen dimensions are %d x %d\n", nrows, ncols);
    }
    
    clear_screen();
    add_background();
    display_screen();
  }
}

bool play_game(char* cmd_str) {
  job* j = jl_get_foreground();
  if (!j) {
    strcpy(error_msg, "Cannot run 'jumper' in the background");
    return false;
  }
  pid_t pid = fork();
  if (pid == 0) {
    jumper();
    exit(0);
  } else {
    job_add_process(j, pid, RUNNING, cmd_str);
    if (setpgid(pid, job_get_gpid(j)) != 0) {
      sprintf(error_msg, "setpgid error: %s", strerror(errno));
      return false;
    }
    return true;
  }
}
