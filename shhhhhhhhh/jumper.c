#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <poll.h>

#include "utils.h"
#include "jumper.h"
#include "job.h"

#define SOLID_SQUARE(c)      ((cell){ .tkn = ' ', .fcol = nocolor, .bcol = c})
#define LETTER(c, col)       ((cell){ .tkn = c, .fcol = col, .bcol = nocolor})

#define PLAYER_TKN           '@'
#define PLAYER_C             (ncols/2)
#define PLAYER_DEF_COL       yellow
#define PLAYER_CELL(p)       ((cell){ .tkn = PLAYER_TKN, .fcol = p.col, .bcol = nocolor})
#define PLAYER_MAX_HEIGHT    5

#define PLAYER_INCLINE 0b00000001

#define OBSTACLE_COL          light_grey
#define OBSTACLE_BASE_PROB    0.01
#define OBSTACLE_PROB_DELTA   0.005
#define OBSTACLE_SPAWN_PROB   (OBSTACLE_BASE_PROB + OBSTACLE_PROB_DELTA * score)
#define OBSTACLE_MAX_HEIGHT   3
#define OBSTACLE_MAX_NUM      6
#define OBSTACLE_MIN_GAP      5

#define GROUND_HEIGHT       8
#define GROUND_PERCENT_DIRT 0.70

#define MIN_NUM_COLS_FOR_SCORE   12
#define SCORE_COLOR              yellow

#define MILLISECOND   1000
#define HALFSECOND    (500*MILLISECOND)

typedef uint8_t flag_t;
typedef uint8_t item_t;
typedef struct {
  char tkn;
  int fcol; // foreground color
  int bcol; // background color
} cell;
typedef struct {
  int c;
  int height;
} obstacle;
typedef struct {
  int r, c;
  int col;
  item_t type;
} item;


static const int nocolor    __attribute__((unused)) = 39;
static const int red        __attribute__((unused)) = 31;
static const int green      __attribute__((unused)) = 32;
static const int yellow     __attribute__((unused)) = 33;
static const int blue       __attribute__((unused)) = 34;
static const int magenta    __attribute__((unused)) = 35;
static const int cyan       __attribute__((unused)) = 36;
static const int white      __attribute__((unused)) = 97;
static const int light_grey __attribute__((unused)) = 37;
static const int dark_grey  __attribute__((unused)) = 90;

static struct termios orig_state;

// I probably don't need any of these default values since static data is zero-initialized anyways
// but better safe than sorry I guess
static vec obstacles = { .cap = 0 };
static vec items = { .cap = 0 };
static int score = 0;
static int obstacle_far_left = 0;

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

  vec* vecs[] = {&screen, &items, &obstacles};
  for (int i = 0; i < sizeof(vecs)/sizeof(vec*); i++) {
    if (vecs[i]->cap != 0) free_vec(vecs[i]);
  }
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

static float randf(float min, float max) {
  return rand()*(max - min)/RAND_MAX + min;
}

// exclusive on upper bound, inclusive on lower bound
static int randi(int min, int max) {
  return rand()*(double)(max-min)/RAND_MAX + min;
}

// My first variadic arguments function
__attribute__((unused))
static void debug_print(char* format, ...) {
  va_list args;

  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  usleep(HALFSECOND);
}

static int row_above_ground() {
  return nrows - GROUND_HEIGHT - 1;
}

static inline void set_screen(int r, int c, cell val) {
  *(cell*)vec_get(&screen, r * ncols + c) = val;
}

static void write_to_screen(int r, int c, char* s, int col) {
  for (int idx = r * ncols + c; s && *s; s++) {
    *(cell*)vec_get(&screen, idx++) = LETTER(*s, col);
  }
}

static inline cell get_screen(int r, int c) {
  return *(cell*)vec_get(&screen, r * ncols + c);
}

// TODO: Figure out why the numbers are so big when using emacs
//       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//       This issue just fixed itself
static void clear_screen() {
  if (screen.cap == 0) screen = vec_new(sizeof(cell), 0, NULL);
  
  struct winsize wsz;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    exit(0xbad);
  }

  nrows = wsz.ws_row;
  ncols = wsz.ws_col;

  vec_resize(&screen, nrows * ncols);
  memset(screen.data, 0, nrows * ncols * sizeof(cell));
}

static void add_background() {
  for (int r = nrows - 1; r >= nrows - GROUND_HEIGHT; --r) {
    int col = nrows - 1 - r < GROUND_HEIGHT * GROUND_PERCENT_DIRT ? blue : light_grey;
    for (int c = 0; c < ncols; c++) {
      set_screen(r, c, SOLID_SQUARE(col));
    }
  }
}

static void add_objects() {
  set_screen(player.r, PLAYER_C, PLAYER_CELL(player));

  int count = 0;
  const int r = row_above_ground();
  for (void* it = vec_first(&obstacles); it; it = vec_next(&obstacles, it)) {
    const obstacle* o = (const obstacle*)it;
    for (int h = 0; h < o->height; h++) {
      set_screen(r - h, o->c, SOLID_SQUARE(OBSTACLE_COL));
    }
    count++;
  }
  //debug_print("Added %d (of %d) obstacles to the screen\n", count, obstacles.size);
}

static void add_score() {
  if (ncols < MIN_NUM_COLS_FOR_SCORE) return;
  if (row_above_ground() <= 0) return;

  char line[MAX_NUM_LEN + sizeof("score: ")];
  sprintf(line, "score: %d", score);
  write_to_screen(0, 0, line, SCORE_COLOR);
}

static void clear_terminal() {
  printf("\e[0m\e[1;1H\e[2J");
  curr_colors.fcol = curr_colors.bcol = 0;
}

static void move_cursor(int line, int col) {
  printf("\033[%d;%dH", line + 1, col + 1);
}

static void draw_cell(cell c) {
  if (curr_colors.fcol != c.fcol || curr_colors.bcol != c.bcol) {
    printf("\e[%d;%dm", c.fcol, c.bcol + 10);
    curr_colors.fcol = c.fcol;
    curr_colors.bcol = c.bcol;
  }
  printf("%c", c.tkn);
}

static void draw_cell_at(int line, int col, cell c) {
  move_cursor(line, col);
  draw_cell(c);
}

static bool skip_cell(cell c) {
  return c.tkn == '\0';
}

static void display_screen() {
  clear_terminal();
  bool skipped = true;
  for (int r = 0; r < nrows; r++) {
    //skipped = true; // for some reason adding colors makes me need this
    for (int c = 0; c < ncols; c++) {
      cell cell = get_screen(r, c);
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
  player.r = row_above_ground();
  player.col = PLAYER_DEF_COL;
  player.state = 0;

  curr_colors.fcol = curr_colors.bcol = nocolor;

  obstacles = vec_new(sizeof(obstacle), 0, NULL);
  items = vec_new(sizeof(item), 0, NULL);
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

static void update_player() {
  if (is_player_jumping()) {
    player.r += player_has_flags(PLAYER_INCLINE) ? -1 : 1;
    if (nrows - GROUND_HEIGHT - 1 - player.r >= PLAYER_MAX_HEIGHT) player_unset_flags(PLAYER_INCLINE);
  }
}

static void update_obstacles() {
  for (int i = 0; i < vec_size(&obstacles); i++) {
    obstacle* o = (obstacle*)vec_get(&obstacles, i);
    if (--o->c < 0) {
      score += o->height;
      swap(o, vec_back(&obstacles), sizeof(obstacle));
      vec_pop(&obstacles);
      i--;
    }
  }
  obstacle_far_left--;
  if (vec_size(&obstacles) == 0 || (randf(0, 1) <= OBSTACLE_SPAWN_PROB &&
                                    vec_size(&obstacles) < OBSTACLE_MAX_NUM - 1 &&
                                    obstacle_far_left < ncols - OBSTACLE_MIN_GAP)) {
    obstacle_far_left = ncols-1;
    obstacle o = { .c = ncols-1, .height = randi(0, OBSTACLE_MAX_HEIGHT) + 1 };
    vec_push(&obstacles, &o);
    //debug_print("Pushed obstacle with height %d (vec size = %d)\n", o.height, vec_size(&obstacles));
  }
}

static void update() {
  update_player();
  update_obstacles();
}

static void display() {
  clear_screen();
  add_background();
  add_objects();
  add_score();
  display_screen();
}

static void jumper() {
  setup_term();
  atexit(cleanup);

  srand(time(NULL));
  clear_screen(); // set nrows, ncols
  init_game();
  for (int key = 0; key != 'q' && key != EOF; key = get_key()) {
    //if (key != 0) debug_print("Screen dimensions are %d x %d\n", nrows, ncols);
    
    handle_input(key);
    update();
    display();
    usleep(100*MILLISECOND);
  }
}

// I forget that this file exists within (code for) a shell
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
