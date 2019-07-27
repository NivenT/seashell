#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
  if (!pipe || !tkns) exit(0xBAD);
  token eoi = {EOI, string_new(NULL)};
  vec_push(tkns, &eoi);
  int size = vec_size(tkns);
  if (size == 1) {
    strcpy(error_msg, "Tried making pipeline out of empty token stream");
    return false;
  }

  pipe->fg = true;
  pipe->cmds = vec_new(sizeof(command), 0, clean_cmd);
  pipe->infile = pipe->outfile = NULL;

  command curr; int arg = 0;
  curr.args[0] = 0;
  
  enum {CMD, ARGS, INPUTFILE, OUTPUTFILE} mode = CMD;
  for (int i = 0; i < size; i++) {
    token* tkn = (token*)vec_get(tkns, i);
    if (tkn->type == SYMBOL || tkn->type == STRING) {
      switch(mode) {
      case CMD:
	if (tkn->type == SYMBOL) {
	  curr.name = tkn->str.cstr;
	} else {
	  strcpy(error_msg, "Each part of the pipeline must begin with a command");
	  return false;
	}
	mode = ARGS;
	break;
      case ARGS:
	curr.args[arg++] = tkn->str.cstr;
	if (arg >= MAX_NUM_ARGS) {
	  strcpy(error_msg, "Exceeded the maximum number of allowed arguments in a command");
	  return false;
	}
	curr.args[arg] = 0;
	break;
      case INPUTFILE:
	pipe->infile = tkn->str.cstr;
	mode = CMD;
	break;
      case OUTPUTFILE:
	pipe->outfile = tkn->str.cstr;
	mode = CMD;
	break;
      }
    } else {
      if (tkn->type == AMPERSAND) {
	pipe->fg = false;
	if (i < size - 2) {
	  strcpy(error_msg, "The ampersand goes at the end of the command");
	  return false;
	}
      } else if (tkn->type == INFILE) {
	mode = INPUTFILE;
      } else if (tkn->type == OUTFILE) {
	mode = OUTPUTFILE;
      } else if (tkn->type == PIPE) {
	mode = CMD;
      }

      if (curr.name) {
	vec_push(&pipe->cmds, &curr);
	curr.name = NULL;
	curr.args[0] = 0;
	arg = 0;
      }
    }
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
	// Should I be checking for dup2 errors?
	dup2(fds[(i-1) << 1], STDIN_FILENO);
      } else if (p.infile) {
	int fd = open(p.infile, O_RDONLY);
	if (fd < 0) {
	  sprintf(error_msg, "Could not open the given input file: %s", p.infile);
	  return false;
	}
	dup2(fd, STDIN_FILENO);
      }

      if (i + 1 < ncmds) {
	dup2(fds[1+(i << 1)], STDOUT_FILENO);
      } else if (p.outfile) {
	int fd = creat(p.outfile, 0644);
	if (fd < 0) {
	  sprintf(error_msg, "Could not create the given output file: %s", p.outfile);
	  return false;
	}
	dup2(fd, STDOUT_FILENO);
      }
	
      // TODO: Abstract away into closeall function
      for (int j = 0; j < nfds; j++) {
	close(fds[j]);
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
  if (pipe->infile) free(pipe->infile);
  if (pipe->outfile) free(pipe->outfile);
}

void print_pipeline(pipeline* pipe) {
  int size = vec_size(&pipe->cmds);
  for (int i = 0; i < size; i++) {
    command* cmd = (command*)vec_get(&pipe->cmds, i);
    char* str = command_to_string(*cmd);
    printf("%s ", str);
    free(str);
    if (i < size - 1) {
      printf("| ");
    }
  }
  if (pipe->infile) printf("< %s ", pipe->infile);
  if (pipe->outfile) printf("> %s ", pipe->outfile);
  if (!pipe->fg) printf("&");
}
