#include <stdlib.h>

#include "job.h"

joblist jobs;

static void free_job(void* data) {
  job j = *(job*)data;
  free_vec(&j.processes);
}

static void cleanup() {
}

void init_jobs() {
  jobs.next = 1;
  jobs.jobs = map_int_new(sizeof(job), 0, free_job);

  atexit(cleanup);
}
