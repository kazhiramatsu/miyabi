#include "miyabi/perl.h"

perl_state *
perl_new(void)
{
  perl_state *state;

  state = malloc(sizeof(perl_state));
  state->top = NULL;
  state->base = NULL;
  state->stack = NULL;
  state->stack_end = NULL;
  state->ci = NULL; 
  state->ci_base = NULL;
  state->options = 0;
  state->filename = NULL;
  state->ci = NULL;
  state->ci_end = NULL;
  state->gc = perl_gc_new(state, 1024*100);

  return state;
}
