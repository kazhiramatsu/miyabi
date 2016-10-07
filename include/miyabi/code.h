#ifndef PERL_CODE_H
#define PERL_CODE_H

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/array.h"

typedef int (*perl_cfunc)(perl_state *state);

struct perl_code {
  perl_object_header
  perl_instruction *code;
  int size;
  perl_array constants;
  _Bool is_imported;
  perl_cfunc cfunc;
};

perl_code perl_code_new(perl_state *state);
void perl_code_emit(perl_code *code);

#endif
