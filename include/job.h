#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include <sys/types.h>
#include <stddef.h>

#include "defs.h"

typedef struct process process;
typedef struct job job;
typedef struct joblist joblist;
typedef enum {RUNNING, STOPPED, WAITING, TERMINATED} procstate;

// Think of a process as a command
struct process {
  pid_t pid;
  procstate state;
  char* cmd;
};

// Think of a job as a pipeline
struct job {
  // job #/id. Couldn't decide which I want to call it
  union {
    size_t num;
    size_t id;
  };
  vec processes;
  bool fg;
};

struct joblist {
  size_t next;
  map jobs; // map from job id (int) to job
  job* foreground;
};

extern void init_jobs();

extern char* procstate_to_string(procstate state);

extern void job_add_process(job* j, pid_t pid, procstate state, char* cmds);
extern pid_t job_get_gpid(job* j);
extern bool job_is_stopped(job* j);
extern bool job_is_terminated(job* j);
extern void job_print(job* j);

// These all implicitly operate on a global joblist
extern job* jl_new_job(bool fg);
extern job* jl_get_job_by_pid(pid_t pid);
extern job* jl_get_job_by_id(size_t id);
extern process* jl_get_proc(pid_t pid);
extern void jl_update_state(pid_t pid, procstate state);
extern procstate jl_get_state(pid_t pid);
extern void jl_print();
extern void jl_remove_job(size_t id);
extern bool jl_has_fg();
extern pid_t jl_fg_gpid();
extern bool jl_resume_first_stopped();

#endif // JOB_H_INCLUDED
