#include <string.h>
#include <stdlib.h>

#include "pipeline.h"
#include "parser.h"

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

    strcpy(curr->cmd.name, tkn->str.cstr);
    int arg = 0;
    // I always love writing lines like this
    while (++idx < size && (tkn = (token*)vec_get(&tkns, idx))->type != PIPE) {
      if (arg >= MAX_NUM_ARGS) {
	strcpy(error_msg, "Exceeded the maximum number of allowed arguments in a command");
	return false;
      }
      // strdup?
      curr->cmd.args[arg++] = tkn->str.cstr;
    }
    if (arg < MAX_NUM_ARGS) curr->cmd.args[arg] = 0;
    idx++;
  }
  return true;
}
