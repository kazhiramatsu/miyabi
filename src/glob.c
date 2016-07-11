#include "miyabi/perl.h"

perl_glob
perl_glob_new(perl_state *state)
{
  struct perl_glob *glob = malloc(sizeof(struct perl_glob));

  glob->base.type = PERL_TYPE_GLOB;
  glob->scalar = perl_undef_new();
  glob->code = perl_undef_new();
  glob->hash = perl_undef_new();
  glob->array = perl_undef_new();
  glob->eglob = perl_undef_new();

  return perl_glob_init(glob);
}

perl_glob
perl_glob_code_add(perl_code code)
{
  struct perl_glob *g = malloc(sizeof(struct perl_glob));

  if (!g->code) {
    g->code = code;
  }
	return perl_glob_init(g);
}

perl_glob
perl_glob_hash_add(perl_hash hash)
{
  struct perl_glob *g = malloc(sizeof(struct perl_glob));

  if (!g->hash) {
    g->hash = hash;
  }
	return perl_glob_init(g);
}

perl_glob
perl_glob_fetch(perl_state *state, perl_scalar name)
{
  perl_scalar *stash = perl_hash_fetch(state, state->defstash, name, perl_undef_new(), false);
  if (stash != NULL) {
  }

}
