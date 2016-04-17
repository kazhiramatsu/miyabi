#ifndef PERL_REF_H
#define PERL_REF_H

#include "miyabi/perl.h"
#include "miyabi/value.h"

typedef struct perl_ref {
  perl_object base;
  perl_value value;
} perl_ref;

#endif
