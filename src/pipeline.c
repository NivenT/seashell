#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "pipeline.h"
#include "parser.h"
#include "builtins.h"

bool build_pipeline(vec tkns, pipeline* pipe) {
  if (!pipe) exit(0xBAD); // This is a programmer error, not a user error
  int size = vec_size(&tkns);
  if (size == 0) {
    strcpy(error_msg, "Tried making pipeline out of empty token stream");
    return false;
  }

  pipeline* curr = NULL;
  
  int idx = 0;
  while (idx < size) {
    if (curr) {
      curr->next = malloc(sizeof(pipeline));
      curr = curr->next;
    } else {
      curr = pipe;
    }
    // could maybe replace with a call to calloc?
    curr->next = NULL;
    
    token* tkn = (token*)vec_get(&tkns, idx);
    if (tkn->type != SYMBOL) {
      strcpy(error_msg, "Each part of the pipeline must begin with a command");
      return false;
    }

    curr->cmd.name = tkn->str.cstr;
    int arg = 0;
    // I always love writing lines like this
    while (++idx < size && (tkn = (token*)vec_get(&tkns, idx))->type != PIPE) {
      if (arg >= MAX_NUM_ARGS) {
	strcpy(error_msg, "Exceeded the maximum number of allowed arguments in a command");
	return false;
      }
      curr->cmd.args[arg++] = tkn->str.cstr;
    }
    if (arg < MAX_NUM_ARGS) curr->cmd.args[arg] = 0;
    idx++;
  }
  return true;
}

bool execute_pipeline(pipeline p, pid_t* last_pid, job* j) {
  bool is_builtin = false;
  if (!handle_builtin(p.cmd, &is_builtin)) {
    return false;
  } else if (!is_builtin) { // Can I make this branch not messy?
    const size_t ncmds = num_cmds(&p);
    const size_t nfds = (ncmds - 1) << 1;
    int fds[nfds]; // I'm surprised this is legal

    for (size_t i = 0; i < nfds >> 1; i++) {
      if (pipe(&fds[i << 1]) < 0) {
	strcpy(error_msg, "Could not create a pipe");
	return false;
      }
    }

    pipeline* curr = &p;
    char* argv[MAX_NUM_ARGS + 2];
    for (size_t i = 0; i < ncmds; i++) {
      *last_pid = fork();
      if (*last_pid == 0) {
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
        
	execvp(curr->cmd.name, (char**)&curr->cmd);
	sprintf(error_msg, "Could not run command %s: %s", curr->cmd.name, strerror(errno));
	return false;
      }

      job_add_process(j, *last_pid, RUNNING, command_to_string(curr->cmd));
      curr = curr->next;
    }

    for (int j = 0; j < nfds; j++) {
      close(fds[j]);
    }
  }
  return true;
}

int num_cmds(const pipeline* pipe) {
  return pipe ? 1 + num_cmds(pipe->next) : 0;
}

void free_pipeline(pipeline* pipe) {
  if (pipe->next) {
    free_pipeline(pipe->next);
    free(pipe->next);
  }
  free_cmd(&pipe->cmd);
}
