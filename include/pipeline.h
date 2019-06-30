#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED

#include "commands.h"

typedef struct pipeline pipeline;

// Would a vec be better than a linked list?
struct pipeline {
  command cmd;
  pipeline* next;
};

bool build_pipeline(vec tkns, pipeline* pipe);

#endif // PIPELINE_H_INCLUDED
