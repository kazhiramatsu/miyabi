#ifndef PERL_IO_H
#define PERL_IO_H

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/str.h"

char *perl_io_gets(perl_state *state, perl_scalar str, FILE *fp);

#endif

