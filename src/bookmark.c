#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bookmark.h"
#include "utils.h"

char* bookmark_file = NULL;

char* get_bookmark_file() {
  if (!bookmark_file) {
    int home_len = strlen(home_dir);
    int len = strlen(BOOKMARK_FILE) + home_len + 1;
    
    bookmark_file = malloc(len+1);

    strcpy(bookmark_file, home_dir);
    bookmark_file[home_len] = '/';
    strcpy(bookmark_file + home_len + 1, BOOKMARK_FILE);
    bookmark_file[len] = '\0';
  }
  return bookmark_file;
}

bool save_bookmark(const char* path, const char* name) {
  int fd = open(get_bookmark_file(), O_WRONLY | O_CREAT | O_APPEND);
  if (fd == -1) {
    strcpy(error_msg, "Could not open bookmark file");
    return false;
  }
  char line[MAX_BM_LINE_LEN];
  sprintf(line, "%s %s\n", path, name);
  
  if (!write_all(fd, line, MAX_BM_LINE_LEN)) {
    strcpy(error_msg, "Could not write to bookmark file");
    return false;
  }
  return true;
}

bool list_bookmarks() {
  int fd = open(get_bookmark_file(), O_RDONLY);
  if (fd == -1) return true;
  FILE* f = fdopen(fd, "r");
  if (!f) return true;
  char line[MAX_PTH_LEN]; // TODO: Technically should be bigger
  for (int l = 1; fgets(line, MAX_CMD_LEN, f); l++) {
    printf("%d %s", l, line);
  }
  return true;
}

bool goto_bookmark(const char* name) {
  return true;
}

