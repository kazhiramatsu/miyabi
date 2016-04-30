#ifndef PERL_STATE_H
#define PERL_STATE_H

#include "miyabi/value.h"

typedef struct perl_state {
  perl_scalar *top;
  perl_scalar *base;
  perl_scalar *stack;
  perl_scalar *stack_end;
  size_t stack_size;
  perl_callinfo *ci;      /* current call info */
  perl_callinfo *ci_end;
  perl_callinfo *ci_base; /* top level ci */
  int argc;
  char **argv;
  uint32_t options;       /* run verbose mode */
  char *filename;         /* program filename */
  struct perl_vm_heap *heaps;
  struct perl_code *toplevel; /* toplevel bytecode */
  perl_hash defstash;
 	perl_hash curstash;
	perl_scalar curstashname;
} perl_state;

#endif
