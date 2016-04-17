#include "miyabi/perl.h"
#include "miyabi/value.h"
#include <assert.h> 

perl_scalar
perl_num_init(double val)
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
perl_int_init(uint64_t val)
{
  return (val & PERL_VALUE_MASK) | PERL_TAG_NAN | PERL_TAG_INT;
}

perl_scalar
perl_str_init(struct perl_str *str)
{
  return (uint64_t)str | PERL_TAG_NAN | PERL_TAG_STR;
}

perl_array
perl_array_init(struct perl_array *array)
{
  return (uint64_t)array | PERL_TAG_NAN | PERL_TAG_ARRAY;
}

perl_hash
perl_hash_init(struct perl_hash *hash)
{
  return (uint64_t)hash | PERL_TAG_NAN | PERL_TAG_HASH;
}

perl_undef
perl_undef_new(void)
{
  return PERL_TAG_NAN | PERL_TAG_UNDEF;
}

perl_code
perl_code_init(struct perl_code *code)
{
  return (uint64_t)code | PERL_TAG_NAN | PERL_TAG_CODE;
}

perl_glob
perl_glob_init(struct perl_glob *glob)
{
  return (uint64_t)glob | PERL_TAG_NAN | PERL_TAG_GLOB;
}

_Bool
perl_undef_p(perl_scalar val)
{
  return perl_type(val) == PERL_TYPE_UNDEF;
}

double
perl_to_num(perl_scalar val)
{
  union {
    double d;
    uint64_t v;
  } u;

	u.v = val;
  return u.d;
}

uint64_t
perl_to_int(perl_scalar val)
{
  return val ^ (PERL_TAG_NAN | PERL_TAG_INT);
}

struct perl_str *
perl_to_str(perl_scalar val)
{
  return (struct perl_str *)(val ^ (PERL_TAG_NAN | PERL_TAG_STR));
}

struct perl_array *
perl_to_array(perl_array val)
{
  return (struct perl_array *)(val ^ (PERL_TAG_NAN | PERL_TAG_ARRAY));
}

struct perl_hash *
perl_to_hash(perl_hash val)
{
  return (struct perl_hash *)(val ^ (PERL_TAG_NAN | PERL_TAG_HASH));
}

struct perl_code *
perl_to_code(perl_code val)
{
  return (struct perl_code *)(val ^ (PERL_TAG_NAN | PERL_TAG_CODE));
}

struct perl_glob *
perl_to_glob(perl_glob val)
{
  return (struct perl_glob *)(val ^ (PERL_TAG_NAN | PERL_TAG_GLOB));
}

void
perl_scalar_dump(perl_scalar v)
{
  switch (perl_type(v)) {
    case PERL_TYPE_STR: {
      perl_str *str;
      str = perl_to_str(v);
      printf("%s", str->str);
      break;
    } 
    case PERL_TYPE_NUM: {
      printf("%lf", perl_to_num(v));
      break;
    }
    case PERL_TYPE_INT: {
      printf("%llu", perl_to_int(v));
      break;
    } 
    case PERL_TYPE_HASH: {
      break;
    }
    case PERL_TYPE_ARRAY: {
      break;
    }
    default:
      printf("unknown data type\n");
  } 
}

