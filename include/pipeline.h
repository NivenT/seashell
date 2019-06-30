#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED

#include "commands.h"

typedef struct pipeline pipeline;

// Would a vec be better than a linked list?
struct pipeline {
  command cmd;
  pipeline* next;
};

extern bool build_pipeline(vec tkns, pipeline* pipe);
// TODO: Replace last_pid with a global struct for keeping track of child info + a SIGCHLD handler
extern bool execute_pipeline(pipeline pipe, pid_t* last_pid);
extern int num_cmds(const pipeline* pipe);

#endif // PIPELINE_H_INCLUDED
