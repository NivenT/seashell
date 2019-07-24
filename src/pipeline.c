#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "pipeline.h"
#include "parser.h"
#include "builtins.h"

void clean_cmd(void* addr) {
  free_cmd((command*)addr);
}

bool build_pipeline(vec* tkns, pipeline* pipe) {
  if (!pipe) exit(0xBAD); // This is a programmer error, not a user error
  int size = vec_size(tkns);
  if (size == 0) {
    strcpy(error_msg, "Tried making pipeline out of empty token stream");
    return false;
  }

  pipe->fg = true;
  if (((token*)vec_get(tkns, size-1))->type == AMPERSAND) {
    pipe->fg = false;
    vec_pop(tkns);
    size--;
  }

  pipe->cmds = vec_new(sizeof(command), 0, clean_cmd);
  
  int idx = 0;
  while (idx < size) {
    command curr;
    token* tkn = (token*)vec_get(tkns, idx);
    if (tkn->type != SYMBOL) {
      strcpy(error_msg, "Each part of the pipeline must begin with a command");
      return false;
    }

    curr.name = tkn->str.cstr;
    int arg = 0;
    // I always love writing lines like this
    while (++idx < size && (tkn = (token*)vec_get(tkns, idx))->type != PIPE) {
      if (arg >= MAX_NUM_ARGS) {
	strcpy(error_msg, "Exceeded the maximum number of allowed arguments in a command");
	return false;
      }
      curr.args[arg++] = tkn->str.cstr;
    }
    if (arg < MAX_NUM_ARGS) curr.args[arg] = 0;
    idx++;

    vec_push(&pipe->cmds, &curr);
  }
  return true;
}

bool execute_pipeline(pipeline p, job* j) {
  const size_t ncmds = num_cmds(&p);
  const size_t nfds = (ncmds - 1) << 1;
  int fds[nfds]; // I'm surprised this is legal

  for (size_t i = 0; i < nfds >> 1; i++) {
    if (pipe(&fds[i << 1]) < 0) {
      strcpy(error_msg, "Could not create a pipe");
      return false;
    }
  }

  char* argv[MAX_NUM_ARGS + 2];
  for (size_t i = 0; i < ncmds; i++) {
    command cmd = *(command*)vec_get(&p.cmds, i);
      
    pid_t pid = fork();
    if (pid == 0) {
      if (i > 0) {
	dup2(fds[(i-1) << 1], STDIN_FILENO);
      }

      if (i + 1 < ncmds) {
	dup2(fds[1+(i << 1)], STDOUT_FILENO);
      }
	
      // TODO: Abstract away into closeall function
      for (int j = 0; j < nfds; j++) {
	close(fds[j]);
      }

      pid_t gpid = job_get_gpid(j);
      if (setpgid(pid, gpid == 0 ? getpid() : gpid) != 0) {
	sprintf(error_msg, "setpgid error (in child): %s", strerror(errno));
	return false;
      }
      execvp(cmd.name, (char**)&cmd);
      sprintf(error_msg, "Could not run command %s: %s", cmd.name, strerror(errno));
      return false;
    }

    job_add_process(j, pid, RUNNING, command_to_string(cmd));
    if (setpgid(pid, job_get_gpid(j)) != 0) {
      sprintf(error_msg, "setpgid error: %s", strerror(errno));
      return false;
    }
  }

  for (int j = 0; j < nfds; j++) {
    close(fds[j]);
  }
  return true;
}

int num_cmds(const pipeline* pipe) {
  return vec_size(&pipe->cmds);
}

void free_pipeline(pipeline* pipe) {
  free_vec(&pipe->cmds);
}
