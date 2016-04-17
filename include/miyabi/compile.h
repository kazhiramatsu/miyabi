#ifndef PERL_COMPILE_H
#define PERL_COMPILE_H

#include "miyabi/perl.h"
#include "miyabi/glob.h"
#include "miyabi/value.h"
#include "miyabi/node.h"
#include "miyabi/parse.h"
#include "miyabi/code.h"

typedef struct compiler_unit compiler_unit;

typedef struct perl_compiler {
  perl_scalar curstashname;
  perl_hash curstash;
  perl_hash defstash;
  perl_state *state;
  compiler_unit *u;
  perl_array stash_stack;
} perl_compiler;

void peep(perl_compiler *c, perl_instruction cur, int reg);
int add_const(perl_compiler *c, perl_node *n);
void compile_sassign(perl_compiler *c, perl_node *first, int reg);
perl_compiler *perl_compiler_new(perl_state *state);
perl_code perl_compile(perl_state *state, perl_node *n);
void perl_code_dump(perl_state *state, perl_code code);

#endif
