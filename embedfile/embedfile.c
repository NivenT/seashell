#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "mystring.h" // from seashell

#define DEF_FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define BLK_SIZE 256

void print_usage(const char* name) {
  printf("Usage:\n");
  printf("%s TEXT_FILE SOURCE_FILE SENTINEL DEST_FILE\n", name);
  printf("\n");
  printf("example:\n");
  printf("%s data.txt code.c FILE_GOES_HERE modified_code.c\n", name);
  exit(1);
}

string readfile(const char* path) {
  string ret = string_new(NULL);
  
  int fd = open(path, O_RDONLY, DEF_FILE_MODE);
  if (fd < 0) return ret;

  char block[BLK_SIZE];
  for (ssize_t bytes; (bytes = read(fd, block, BLK_SIZE)) > 0; string_appendn(&ret, block, bytes));
  
  return ret;
}

void writefile(const char* path, const string* text) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, DEF_FILE_MODE);
  if (fd < 0) return;
  for (int pos = 0, num; (num = write(fd, text->cstr + pos, text->len - pos)) > 0; pos += num);
}

void replace_all(string* str, const char* old, const char* new) {
  const int old_len = strlen(old);
  const int new_len = strlen(new);
  for (int pos = 0; (pos = string_find(str, pos, old)) != -1; pos += new_len) {
    string_replace(str, pos, old_len, new);
  }
}

void ceeify(string* str) {
  replace_all(str, "\\", "\\\\");
  replace_all(str, "\r\n", "\\n");
  replace_all(str, "\n", "\\n");
  replace_all(str, "\"", "\\\"");
}

int main(int argc, char* argv[]) {
  if (argc != 5) print_usage(argv[0]);

  string text = readfile(argv[1]);
  ceeify(&text);
  string source = readfile(argv[2]);
  replace_all(&source, argv[3], text.cstr);
  writefile(argv[4], &source);

  free_string(&source);
  free_string(&text);
  return 0;
}
