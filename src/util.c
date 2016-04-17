
#include "miyabi/perl.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void *
perl_saveptr(void *s, unsigned long len)
{
  void *newaddr = malloc(len);

  memcpy(newaddr, s, len);
  return newaddr;
}

void
perl_warn(perl_state *state, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

void
perl_croak(perl_state *state, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

