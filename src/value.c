#include "miyabi/perl.h"

perl_scalar
perl_num_make(double val)
{
  union {
    double d;
    uint64_t v;
  } u;

  if (isnan(val)) {
    return PERL_TAG_NAN;
  }
  u.d = val;
  return u.v;
}

perl_scalar
perl_int_make(uint64_t val)
{
  return (val & PERL_PTR_MASK) | PERL_TAG_NAN | PERL_TAG_INT;
}

perl_scalar
perl_str_make(struct perl_str *str)
{
  return (uint64_t)str | PERL_TAG_NAN | PERL_TAG_STR;
}

perl_array
perl_array_make(struct perl_array *array)
{
  return (uint64_t)array | PERL_TAG_NAN | PERL_TAG_ARRAY;
}

perl_hash
perl_hash_make(struct perl_hash *hash)
{
  return (uint64_t)hash | PERL_TAG_NAN | PERL_TAG_HASH;
}

perl_undef
perl_undef_new(void)
{
  return PERL_TAG_NAN | PERL_TAG_UNDEF;
}

perl_code
perl_code_make(struct perl_code *code)
{
  return (uint64_t)code | PERL_TAG_NAN | PERL_TAG_CODE;
}

perl_glob
perl_glob_make(struct perl_glob *glob)
{
  return (uint64_t)glob | PERL_TAG_NAN | PERL_TAG_GLOB;
}

_Bool
perl_undef_p(perl_scalar val)
{
  return perl_type(val) == PERL_TAG_UNDEF;
}

_Bool
perl_num_p(perl_scalar val)
{
  return !!perl_type(val);
}

double
perl_num_value(perl_scalar val)
{
  union {
    double d;
    uint64_t v;
  } u;

  u.v = val;
  return u.d;
}

uint64_t
perl_int_value(perl_scalar val)
{
  return val ^ (PERL_TAG_NAN | PERL_TAG_INT);
}

struct perl_str *
perl_str_value(perl_scalar val)
{
  return (struct perl_str *)(val ^ (PERL_TAG_NAN | PERL_TAG_STR));
}

struct perl_array *
perl_array_value(perl_array val)
{
  return (struct perl_array *)(val ^ (PERL_TAG_NAN | PERL_TAG_ARRAY));
}

struct perl_hash *
perl_hash_value(perl_hash val)
{
  return (struct perl_hash *)(val ^ (PERL_TAG_NAN | PERL_TAG_HASH));
}

struct perl_code *
perl_code_value(perl_code val)
{
  return (struct perl_code *)(val ^ (PERL_TAG_NAN | PERL_TAG_CODE));
}

struct perl_glob *
perl_glob_value(perl_glob val)
{
  return (struct perl_glob *)(val ^ (PERL_TAG_NAN | PERL_TAG_GLOB));
}

void
perl_scalar_dump(perl_scalar v)
{
  switch (perl_type(v)) {
    default: {
      printf("%lf", perl_num_value(v));
      break;
    }
    case PERL_TAG_STR: {
      struct perl_str *str;
      str = perl_str_value(v);
      printf("%s", str->str);
      break;
    } 
    case PERL_TAG_INT: {
      printf("%llu", perl_int_value(v));
      break;
    } 
    case PERL_TAG_HASH: {
      perl_hash_dump(v);
      break;
    }
    case PERL_TAG_ARRAY: {
      perl_array_dump(v);
      break;
    }
  } 
}

