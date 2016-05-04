#ifndef PERL_COMPILE_H
#define PERL_COMPILE_H

#include "miyabi/perl.h"
#include "miyabi/glob.h"
#include "miyabi/value.h"
#include "miyabi/node.h"
#include "miyabi/parse.h"
#include "miyabi/code.h"

typedef struct compiler_context compiler_context;

typedef struct perl_compiler {
  perl_scalar curstashname;
  perl_hash curstash;
  perl_hash defstash;
  perl_state *state;
  perl_array stash_stack;
  perl_code code;
} perl_compiler;

perl_compiler *perl_compiler_new(perl_state *state);
perl_code perl_compile(perl_state *state, perl_node *n);
void perl_code_dump(perl_state *state, perl_code code);

#endif
