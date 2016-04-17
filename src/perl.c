#include "miyabi/perl.h"

perl_state *
perl_new(void)
{
  perl_state *state;

  state = malloc(sizeof(perl_state));
  state->top = NULL;
  state->base = NULL;
  state->stack = NULL;
  state->stack_last = NULL;
  state->ci = NULL; 
  state->base_ci = NULL;
  state->options = 0;
  state->filename = NULL;

  return state;
}
