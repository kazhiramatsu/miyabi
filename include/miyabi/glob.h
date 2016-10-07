#ifndef PERL_GLOB_H
#define PERL_GLOB_H

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/code.h"

struct perl_glob {
  perl_object_header
	perl_scalar scalar;
  perl_code code;		      /* subroutine value */
  perl_hash hash;		      /* hash value */
  perl_array array;		    /* array value */
  perl_glob eglob;				/* effective gv, if *glob */
};

perl_glob perl_glob_new(perl_state *state); 
perl_glob perl_glob_code_add(perl_code code);
perl_glob perl_glob_hash_add(perl_hash hash);
perl_glob perl_glob_fetch(perl_state *state, perl_scalar name);

#endif
