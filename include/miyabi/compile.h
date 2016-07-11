#ifndef PERL_COMPILE_H
#define PERL_COMPILE_H

#include "miyabi/perl.h"
#include "miyabi/glob.h"
#include "miyabi/value.h"
#include "miyabi/node.h"
#include "miyabi/parse.h"
#include "miyabi/code.h"

typedef struct perl_compiler perl_compiler;

perl_compiler *perl_compiler_new(perl_state *state, perl_compiler *prev, node *n);
struct perl_code *perl_compile(perl_state *state, perl_node *n);
void perl_code_dump(perl_state *state, struct perl_code *c);

#endif
