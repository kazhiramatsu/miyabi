#include "miyabi/perl.h"

perl_code
perl_code_new(perl_state *state)
{
  struct perl_code *code;

  code = malloc(sizeof(struct perl_code));
  code->base.header.type = PERL_TYPE_CODE;
  code->size = 0;
  code->code = NULL;
  code->constants = perl_array_new(state);
  code->cfunc = NULL;

  return perl_code_init(code);
}

