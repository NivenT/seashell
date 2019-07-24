#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED

#include "commands.h"
#include "job.h"

typedef struct pipeline pipeline;

struct pipeline {
  /*
  command cmd;
  pipeline* next;
  */
  vec cmds;
  bool fg;
};

extern bool build_pipeline(vec* tkns, pipeline* pipe);
extern bool execute_pipeline(pipeline pipe, job* j);
extern int num_cmds(const pipeline* pipe);
extern void free_pipeline(pipeline* pipe);

#endif // PIPELINE_H_INCLUDED
