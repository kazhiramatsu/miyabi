#include "miyabi/perl.h"

perl_scalar
perl_str_new(perl_state *state, const char *s, int len)
{
  struct perl_str *str;

  char *buf = malloc(len+1);
  memcpy(buf, s, len);
  buf[len] = '\0';

  str = malloc(sizeof(struct perl_str));
  str->base.type = PERL_TYPE_STR;
  str->str = buf;
  str->fill = len;
  str->max = len+1;
  str->hash = 0;
  
  return perl_str_init(str);
}

perl_scalar
perl_str_grow(perl_state *state, perl_scalar str, int newlen)
{
  struct perl_str *s = perl_to_str(str);

  s->str = realloc(s->str, newlen+1);
  s->fill = newlen;
  s->max = newlen+1;
  return perl_str_init(s);
}

perl_scalar
perl_str_gets(perl_state *state, perl_scalar str, FILE *f)
{
  struct perl_str *s = perl_to_str(str);
  int c;

  for (;;) {
    c = fgetc(f);
    if (c == EOF) {
        break;
    }
    perl_str_putc(state, str, c);
    if (c == '\n') {
      ungetc(c, f);
      break;
    }
  }
  return perl_str_init(s);
}

perl_scalar
perl_str_clear(perl_state *state, perl_scalar str)
{
  struct perl_str *s = perl_to_str(str);

  memset(s->str, '\0', s->fill);
  s->fill = 0;
  return perl_str_init(s);
}

perl_scalar
perl_str_putc(perl_state *state, perl_scalar str, char c)
{
  struct perl_str *s = perl_to_str(str);
  int newlen = s->fill + 1;

  s->str = realloc(s->str, newlen+1);
  s->str[s->fill] = c;
  s->fill = newlen;
  s->max = newlen+1;
  s->str[s->fill] = '\0';
  return perl_str_init(s);
}

perl_scalar 
perl_str_cat(perl_state *state, perl_scalar dstr, perl_scalar sstr)
{
  struct perl_str *ss = perl_to_str(sstr);

  perl_str_cat_cstr(state, dstr, ss->str, ss->fill);
  return dstr;
}

perl_scalar
perl_str_cat_cstr(perl_state *state, perl_scalar str, char *sstr, int len)
{
  struct perl_str *s = perl_to_str(str);
  int newlen = s->fill + len;

  s->str = realloc(s->str, newlen+1);
  memcpy(s->str+s->fill, sstr, len);
  s->fill = newlen;
  s->max = newlen+1;
  s->str[s->fill] = '\0';
  return perl_str_init(s);
}

_Bool
perl_str_eq(perl_state *state, perl_scalar str1, perl_scalar str2)
{
  struct perl_str *s1 = perl_to_str(str1);  
  struct perl_str *s2 = perl_to_str(str2);  

  if (s1->fill != s2->fill) {
    return false;
  }
  for (int i = 0; i < s1->fill; i++) {
    if (s1->str[i] != s2->str[i]) {
      return false;
    }
  }
  return true;
}

perl_scalar
perl_str_copy(perl_state *state, perl_scalar str)
{
  struct perl_str *s = perl_to_str(str);

  return perl_str_new(state, s->str, s->fill);
}

