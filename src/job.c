#include <stdlib.h>
#include <stdio.h>

#include "job.h"

joblist jobs;

static void free_job(void* data) {
  job j = *(job*)data;
  free_vec(&j.processes);
}

static void free_process(void* data) {
  process p = *(process*)data;
  free(p.cmd);
}

static void cleanup() {
  free_map(&jobs.jobs);
}

void init_jobs() {
  jobs.next = 1;
  jobs.jobs = map_int_new(sizeof(job), 0, free_job);

  atexit(cleanup);
}

char* procstate_to_string(procstate state) {
  switch(state) {
  case RUNNING: return "RUNNING";
  case STOPPED: return "STOPPED";
  case WAITING: return "WAITING";
  case TERMINATED: return "TERMINIATED";
  }
}

void job_add_process(job* j, pid_t pid, procstate state, char* cmds) {
  if (!j) return;
  process proc = {pid, state, cmds};
  vec_push(&j->processes, &proc);
}

pid_t job_get_gpid(job* j) {
  if (!j) return 0;
  if (vec_size(&j->processes) == 0) return 0;
  return ((process*)vec_get(&j->processes, 0))->pid;
}

job* jl_new_job(bool fg) {
  job j;
  j.id = jobs.next;
  j.fg = fg;
  j.processes = vec_new(sizeof(process), 0, free_process);
  
  map_insert(&jobs.jobs, &jobs.next, &j);
  job* ret = (job*)map_get(&jobs.jobs, &jobs.next);
  jobs.next++;

  return ret;
}

job* jl_get_job_by_pid(pid_t pid) {
  for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j) {
      for (int idx = 0; idx < vec_size(&j->processes); ++idx) {
	process* proc = (process*)vec_get(&j->processes, idx);
	if (proc->pid == pid) return j;
      }
    }
  }
  return NULL;
}

job* jl_get_job_by_id(size_t id) {
  for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j && j->id == id) return j;
  }
  return NULL;
}

process* jl_get_proc(pid_t pid) {
    for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j) {
      for (int idx = 0; idx < vec_size(&j->processes); ++idx) {
	process* proc = (process*)vec_get(&j->processes, idx);
	if (proc->pid == pid) return proc;
      }
    }
  }
  return NULL;
}

void jl_update_state(pid_t pid, procstate state) {
  process* proc = jl_get_proc(pid);
  if (proc) proc->state = state;
}

procstate jl_get_sate(pid_t pid) {
  process* proc = jl_get_proc(pid);
  return proc ? proc->pid : 0;
}

void jl_print() {
  for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j) {
      printf("[%ld]", j->id);
      for (int idx = 0; idx < vec_size(&j->processes); ++idx) {
	process* proc = (process*)vec_get(&j->processes, idx);
	printf("\t%s\t%s\n", procstate_to_string(proc->state), proc->cmd);
      }
    }
  }
}

void jl_remove_job(size_t id) {
  // Is this valid?
  map_remove(&jobs.jobs, jl_get_job_by_id(id));
}
