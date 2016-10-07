#ifndef PERL_ARRAY_H
#define PERL_ARRAY_H

#include "miyabi/perl.h"
#include "miyabi/value.h"

struct perl_array {
  perl_object_header
  ssize_t fill;
  ssize_t max;
  int head;
  perl_value *array;
};

perl_array perl_array_new(perl_state *state);
perl_scalar perl_array_shift(perl_state *state, perl_array ary);
perl_scalar perl_array_pop(perl_state *state, perl_array ary);
perl_scalar *perl_array_fetch(perl_state *state, perl_array ary, int key, perl_value val, _Bool add);
void perl_array_push(perl_state *state, perl_array ary, perl_scalar value);
perl_scalar *perl_array_store(perl_state *state, perl_array ary, int key, perl_scalar value);
int perl_array_length(perl_state *state, perl_array ary);
void perl_array_dump(perl_array ary);

#endif
