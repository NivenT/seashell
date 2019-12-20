#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

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
  jobs.foreground = NULL;
  jobs.next = 0;
}

void init_jobs() {
  jobs.next = 1;
  jobs.jobs = map_int_new(sizeof(job), 0, free_job);
  jobs.foreground = NULL;
  
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
  int size = vec_size(&j->processes);
  for (int i = 0; i < size; i++) {
    process* p = (process*)vec_get(&j->processes, i);
    return p->pid;
  }
  return 0;
}

// TODO: Consolidate the following three into one function
bool job_is_stopped(job* j) {
  if (!j) return false;
  int size = vec_size(&j->processes);
  for (int i = 0; i < size; i++) {
    process* p = (process*)vec_get(&j->processes, i);
    if (p->state == STOPPED) return true;
  }
  return false;
}

bool job_is_terminated(job* j) {
  if (!j) return true;
  int size = vec_size(&j->processes);
  for (int i = 0; i < size; i++) {
    process* p = (process*)vec_get(&j->processes, i);
    if (p->state != TERMINATED) return false;
  }
  return true;
}

void job_print(job* j) {
  printf("[%ld]", j->id);
  for (int idx = 0; idx < vec_size(&j->processes); ++idx) {
    process* proc = (process*)vec_get(&j->processes, idx);
    printf("\t%s\t%s\n", procstate_to_string(proc->state), proc->cmd);
  }
}

static bool jl_set_foreground(job* j) {
  jobs.foreground = j;
  j->fg = true;
  if (tcsetpgrp(STDIN_FILENO, job_get_gpid(j)) != 0) {
    strcpy(error_msg, "Could not transfer control of the terminal to new job");
    return false;
  }
  return true;
}

job* jl_new_job(bool fg) {
  if (jobs.foreground) return NULL;
  
  job j;
  j.id = jobs.next;
  j.fg = fg;
  j.processes = vec_new(sizeof(process), 0, free_process);
  j.exit_status = -1;
  
  map_insert(&jobs.jobs, &jobs.next, &j);
  job* ret = (job*)map_get(&jobs.jobs, &jobs.next);
  jobs.next++;

  return fg ? (jobs.foreground = ret) : ret;
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

bool jl_has_job(size_t id) {
  return map_get(&jobs.jobs, &id) != NULL;
}

void jl_update_state(pid_t pid, procstate state) {
  job* j = jl_get_job_by_pid(pid);
  process* proc = jl_get_proc(pid);
  if (j && proc) {
    proc->state = state;
    switch(proc->state) {
    case STOPPED:
      j->fg = false;
      jobs.foreground = NULL;
      break;
    }
    if (job_is_terminated(j)) {
      jl_remove_job(j->id);
    }
  }
}

procstate jl_get_sate(pid_t pid) {
  process* proc = jl_get_proc(pid);
  return proc ? proc->state : 0;
}

void jl_print() {
  for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j) job_print(j);
  }
}

void jl_remove_job(size_t id) {
  job* j = jl_get_job_by_id(id);
  if (j == jobs.foreground) jobs.foreground = NULL;
  // Is this valid?
  if (j) map_remove(&jobs.jobs, &j->id);
}

bool jl_has_fg() {
  return jobs.foreground != NULL;
}

pid_t jl_fg_gpid() {
  return jobs.foreground ? job_get_gpid(jobs.foreground) : 0;
}

bool jl_resume_first_stopped() {
  if (jobs.foreground) jobs.foreground->fg = false;
  jobs.foreground = NULL;
  for (int i = 0; i < jobs.next; ++i) {
    job* j = (job*)map_get(&jobs.jobs, &i);
    if (j && job_is_stopped(j)) {
      return jl_resume(j, true);
    }
  }
  strcpy(error_msg, "There is no stopped process");
  return false;
}

bool jl_resume(job* j, bool fg) {
  pid_t gpid = job_get_gpid(j);
  if (gpid <= 0) {
    strcpy(error_msg, "Could not get group pid of this job");
    return false;
  }
  kill(-gpid, SIGCONT);
  return fg ? jl_set_foreground(j) : true;
}
