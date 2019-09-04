#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bookmark.h"
#include "utils.h"

char* bookmark_file = NULL;

static void cleanup() {
  if (bookmark_file) free(bookmark_file);
}

char* get_bookmark_file() {
  if (!bookmark_file) {
    // TODO: Replace below with call to concat_many
    int home_len = strlen(home_dir);
    int len = strlen(BOOKMARK_FILE) + home_len + 1;
    
    bookmark_file = malloc(len+1);
    atexit(cleanup);

    strcpy(bookmark_file, home_dir);
    bookmark_file[home_len] = '/';
    strcpy(bookmark_file + home_len + 1, BOOKMARK_FILE);
    bookmark_file[len] = '\0';
  }
  return bookmark_file;
}

bool save_bookmark(const char* path, const char* name) {
  // TODO: Remember what this 0644 does
  int fd = open(get_bookmark_file(), O_RDWR | O_CREAT | O_APPEND, 0644);
  if (fd == -1) {
    strcpy(error_msg, "Could not open bookmark file");
    return false;
  }
  char line[MAX_BM_LINE_LEN];
  sprintf(line, "%s %s\n", path, name);

  printf("%s", line);
  if (!write_all(fd, line, strlen(line))) {
    close(fd);
    strcpy(error_msg, "Could not write to bookmark file");
    return false;
  }
  close(fd);
  return true;
}

bool list_bookmarks() {
  int fd = open(get_bookmark_file(), O_RDONLY);
  if (fd == -1) return true;
  FILE* f = fdopen(fd, "r");
  if (!f) {
    close(fd);
    return true;
  }
  char line[MAX_PTH_LEN]; // TODO: Technically should be bigger
  for (int l = 1; fgets(line, MAX_PTH_LEN, f); l++) {
    printf("%d %s", l, line);
  }
  fclose(f);
  return true;
}

bool goto_bookmark(const char* name) {
  int fd = open(get_bookmark_file(), O_RDONLY);
  if (fd == -1) return true;
  FILE* f = fdopen(fd, "r");
  if (!f) {
    close(fd);
    return true;
  }
  char line[MAX_PTH_LEN];
  for (int l = 1; fgets(line, MAX_CMD_LEN, f); l++) {
    char* trimmed = trim(line);
    char* backup = trimmed;
    
    char num[10];
    sprintf(num, "%d", l);
    for (char* tkn = num; tkn; tkn = strsep(&trimmed, " ")) {
      if (strcmp(tkn, name) == 0) {
	chdir(strsep(&backup, " ")); // TODO: Check for errors
	goto FUNC_END;
      }
    }
  }
 FUNC_END:
  fclose(f);
  return true;
}

