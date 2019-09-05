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
#include "utils.h"

extern void clean_vec(void*);
extern size_t hash_int(void*);

#ifdef CURL_FOUND
static CURL* curl;
#endif
static map repos; // user -> vec of repos

// https://stackoverflow.com/questions/2624192/good-hash-function-for-strings
static size_t hash_char_star(void* data) {
  char* str = *(char**)data;
  size_t hash = 7;
  while(str && *str) hash = hash*31 + *(str++);
  return hash;
}

static bool char_star_eq(void* lhs, void* rhs) {
  return strcmp(*(char**)lhs, *(char**)rhs) == 0;
}

static void clean_char_star(void* addr) {
  free(*(char**)addr);
}

static void cleanup() {
  #ifdef CURL_FOUND
  if (curl) curl_easy_cleanup(curl);
  #endif
  free_map(&repos);
}

static size_t grow_string(char* ptr, size_t _size, size_t nmemb, void* userdata) {
  size_t size = _size * nmemb;
  string* str = (string*)userdata;
  string_appendn(str, ptr, size);
  return nmemb;
}

void init_networking() {
  #ifdef CURL_FOUND
  curl = curl_easy_init();
  #endif
  repos = map_new(sizeof(vec), sizeof(char*), 16, hash_char_star, char_star_eq,
		  clean_vec, clean_char_star);
  atexit(cleanup);
}

string http_simple_get(const char* url) {
  string ret = string_new(NULL);
  #ifdef CURL_FOUND
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ret);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, grow_string);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  if (curl_easy_perform(curl) != CURLE_OK) {
    free_string(&ret);
    ret = string_new(NULL);
  }
  #endif
  return ret;
}

vec* get_repos(char* user) {
  if (!map_contains(&repos, &user)) {
    const char* strs[] = {"https://api.github.com/users/", user, "/repos", NULL};
    char* url = concat_many(strs);

    vec v = vec_new(sizeof(char*), 0, clean_char_star);
    #ifdef CURL_FOUND
    string data = http_simple_get(url);
    if (data.cstr) {
      char* iter = data.cstr;
      while(true) {
	char* line = strsep(&iter, "\n");
	if (!line) break;
	if (!(line = strstr(line, "full_name"))) continue;
	line += 14 + strlen(user); // before this, line looks like full_name": "<user>/<repo>",
	line[strlen(line)-2] = '\0'; // after this, line should look like <repo>

	char* repo = strdup(line);
	vec_push(&v, &repo);
      }
    }
    free_string(&data);
    #endif
    char* u = strdup(user);
    map_insert(&repos, &u, &v);
    free(url);
  }
  return (vec*)map_get(&repos, &user);
}
