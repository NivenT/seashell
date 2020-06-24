#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <poll.h>

#include "jumper.h"
#include "job.h"

#define SOLID_SQUARE(c) ((Cell){ .tkn = ' ', .fcol = nocolor, .bcol = c})
#define PLAYER_TKN   '@'
#define PLAYER_C     (ncols/2)
#define PLAYER_CELL(p) ((Cell){ .tkn = PLAYER_TKN, .fcol = p.col, .bcol = nocolor})
#define PLAYER_MAX_HEIGHT 3

#define PLAYER_INCLINE 0b00000001

#define GROUND_HEIGHT 8
#define GROUND_PERCENT_DIRT 0.80

#define MILLISECOND   1000
#define HALFSECOND    (500*MILLISECOND)

typedef uint8_t flag_t;
typedef struct Cell {
  char tkn;
  int fcol; // foreground color
  int bcol; // background color
} Cell;

static const int nocolor __attribute__((unused)) = 39;
static const int red __attribute__((unused)) = 31;
static const int green __attribute__((unused)) = 32;
static const int yellow __attribute__((unused)) = 33;
static const int blue __attribute__((unused)) = 34;
static const int magenta __attribute__((unused)) = 35;
static const int cyan __attribute__((unused)) = 36;
static const int white __attribute__((unused)) = 37;

static struct termios orig_state;
static vec screen = { .cap = 0 };
static int nrows, ncols;

static struct {
  int r;
  int col;
  flag_t state; // bitmask
} player;

static struct {
  int fcol;
  int bcol;
} curr_colors;

static void cleanup() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_state);
  printf("\e[0m"); // reset color
}

#include <assert.h>
static void setup_term() {
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
}

static bool kbhit() {
  struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
  if (poll(&pfd, 1, 1) <= 0) return false;
  return pfd.revents & POLLIN;
}

static int get_key() {
  return kbhit() ? getchar() : 0;
}

static inline void set_screen(int r, int c, Cell val) {
  *(Cell*)vec_get(&screen, r * ncols + c) = val;
}

static inline Cell get_screen(int r, int c) {
  return *(Cell*)vec_get(&screen, r * ncols + c);
}

// TODO: Figure out why the numbers are so big when using emacs
//       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//       This issue just fixed itself
static void clear_screen() {
  if (screen.cap == 0) screen = vec_new(sizeof(Cell), 0, NULL);
  
  struct winsize wsz;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    exit(0xbad);
  }

  nrows = wsz.ws_row;
  ncols = wsz.ws_col;

  vec_resize(&screen, nrows * ncols);
  memset(screen.data, 0, nrows * ncols * sizeof(Cell));
}

static void add_background() {
  for (int r = nrows - 1; r >= nrows - GROUND_HEIGHT; --r) {
    int col = r + 1 - nrows < GROUND_HEIGHT * GROUND_PERCENT_DIRT ? red : green;
    for (int c = 0; c < ncols; c++) {
      set_screen(r, c, SOLID_SQUARE(col));
    }
  }
}

static void add_objects() {
  set_screen(player.r, PLAYER_C, PLAYER_CELL(player));
}

static void clear_terminal() {
  printf("\e[1;1H\e[2J");
  //for (int i = 0; i < nrows; i++) printf("\n");
}

static void move_cursor(int line, int col) {
  printf("\033[%d;%dH", line + 1, col + 1);
}

static void draw_cell(Cell c) {
  if (curr_colors.fcol != c.fcol || curr_colors.bcol != c.bcol) {
    printf("\e[%d;%dm", c.fcol, c.bcol + 10);
    curr_colors.fcol = c.fcol;
    curr_colors.bcol = c.bcol;
  }
  printf("%c", c.tkn);
}

static void draw_cell_at(int line, int col, Cell c) {
  move_cursor(line, col);
  draw_cell(c);
}

static bool skip_cell(Cell c) {
  return c.tkn == '\0';
}

static void display_screen() {
  clear_terminal();
  bool skipped = true;
  for (int r = 0; r < nrows; r++) {
    //skipped = true; // for some reason adding colors makes me need this
    for (int c = 0; c < ncols; c++) {
      Cell cell = get_screen(r, c);
      // I love writing trash code
      // (unless you're a potential employer looking at one of my projects.
      //  In that case, I didn't write this and I don't know how it got here.)
      if (skip_cell(cell) && (skipped = true)) continue;
      if (skipped) draw_cell_at(r, c, cell); else draw_cell(cell);
      skipped = false;
    }
  }
  fflush(stdout);
}

static void init_game() {
  player.r = nrows - GROUND_HEIGHT - 1;
  player.col = blue;
  player.state = 0;

  curr_colors.fcol = curr_colors.bcol = nocolor;
}

static bool player_has_flags(flag_t flags) {
  return (player.state & flags) > 0;
}

static void player_set_flags(flag_t flags) {
  player.state |= flags;
}

static void player_unset_flags(flag_t flags) {
  player.state &= ~flags;
}

static bool is_player_jumping() {
  return player.r != nrows - GROUND_HEIGHT - 1 || player_has_flags(PLAYER_INCLINE);
}

static void handle_input(int key) {
  if (key == ' ' && !is_player_jumping()) player_set_flags(PLAYER_INCLINE);
}

static void update() {
  if (is_player_jumping()) {
    player.r += player_has_flags(PLAYER_INCLINE) ? -1 : 1;
    if (nrows - GROUND_HEIGHT - 1 - player.r >= PLAYER_MAX_HEIGHT) player_unset_flags(PLAYER_INCLINE);
  }
}

static void display() {
  clear_screen();
  add_background();
  add_objects();
  display_screen();
}

static void jumper() {
  setup_term();
  atexit(cleanup);

  clear_screen(); // set nrows, ncols
  init_game();
  for (int key = 0; key != 'q' && key != EOF; key = get_key()) {
    /*
    if (key != 0) {
      printf("Screen dimensions are %d x %d\n", nrows, ncols);
      usleep(HALFSECOND);
    }
    */
    handle_input(key);
    update();
    display();
    usleep(100*MILLISECOND);
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
