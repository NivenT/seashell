#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef CURL_FOUND
#include <curl/curl.h>
#endif

#include "networking.h"

/*
static bool connect_to_server(char* host, unsigned short port, int* s) {
  *s = -1;
  struct hostent *he = gethostbyname(host);
  if (!he) return false;

  *s = socket(AF_INET, SOCK_STREAM, 0);
  if (*s < 0) return false;

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = ((struct in_addr*)he->h_addr)->s_addr;

  if (connect(*s, (struct sockaddr*)&server, sizeof(server)) == 0) return true;
  close(*s);
  *s = -1;
  return false;
}

int http_simple_get(char* host, char* path, unsigned short port) {
  int s;
  if (!connect_to_server(host, port, &s)) return -1;
  dprintf(s, "GET %s HTTP/1.0\r\n", path);
  dprintf(s, "HOST: %s\r\n", host);
  dprintf(s, "Upgrade-Insecure-Requests: 0\r\n");
  dprintf(s, "\r\n");
  return s;
}
*/

static CURL *curl;

static void cleanup() {
  curl_easy_cleanup(curl);
}

static size_t grow_string(char* ptr, size_t _size, size_t nmemb, void* userdata) {
  size_t size = _size * nmemb;
  string* str = (string*)userdata;
  string_appendn(str, ptr, size);
  return size;
}

void init_networking() {
  curl = curl_easy_init();
  if (curl) atexit(cleanup);
}

string http_simple_get(const char* url) {
  CURLcode res;

  string ret = string_new(NULL);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ret);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, grow_string);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  if ((res = curl_easy_perform(curl)) != CURLE_OK) {
    free_string(&ret);
    printf("failed: %s\n", curl_easy_strerror(res));
    ret = string_new(NULL);
  }
  return ret;
}
