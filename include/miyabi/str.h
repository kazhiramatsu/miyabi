#ifndef PERL_STR_H
#define PERL_STR_H

#include "miyabi/perl.h"
#include "miyabi/value.h"

typedef struct perl_str {
  perl_object base;
  int hash;
  char *str;
  ssize_t fill;
  ssize_t max;
} perl_str;

perl_scalar perl_str_new(perl_state *state, const char *s, int len);
perl_scalar perl_str_grow(perl_state *state, perl_scalar s, int newlen);
perl_scalar perl_str_gets(perl_state *state, perl_scalar s, FILE *f);
perl_scalar perl_str_clear(perl_state *state, perl_scalar str);
perl_scalar perl_str_putc(perl_state *state, perl_scalar s, char c);
perl_scalar perl_str_cat_cstr(perl_state *state, perl_scalar s, char *str, int len);
_Bool perl_str_eq(perl_state *state, perl_scalar str1, perl_scalar str2);
perl_scalar perl_str_copy(perl_state *state, perl_scalar s);
perl_scalar perl_str_cat(perl_state *state, perl_scalar dstr, perl_scalar sstr);

#endif
