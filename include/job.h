#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include <sys/types.h>
#include <stddef.h>

#include "defs.h"

typedef enum {RUNNING, STOPPED, WAITING, TERMINATED} procstate;

// Think of a job as a pipeline and a process as a command
struct job {
  struct process {
    pid_t pid;
    procstate state;
    char* cmd;
  };
  
  // job #/id. Couldn't decide which I want to call it
  union {
    size_t num;
    size_t id;
  };
  vec processes;
  bool foreground;
};

struct joblist {
  size_t next;
  map jobs;
};

#endif // JOB_H_INCLUDED
