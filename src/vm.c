
#include "miyabi/vm.h"

perl_callinfo *
ci_new(perl_state *state)
{
  if (state->ci == NULL) {
    perl_callinfo *ci = malloc(sizeof(perl_callinfo)*8);
    state->ci = ci;
    state->ci_base = ci;
    state->ci_end = ci+8;
  } else if (state->ci == state->ci_end) {
    size_t cur = (state->ci_end-state->ci);
    size_t new_size = cur*2;
    state->ci_base = realloc(state->ci_base, sizeof(perl_callinfo) * new_size);
    state->ci_end = state->ci_base+new_size;
    state->ci = state->ci_base + cur;
  }
  return state->ci++;
}

void
perl_precall(perl_state *state, perl_code *code)
{
  perl_callinfo *ci;
  perl_scalar *base;
  size_t n;

  n = (size_t)(state->top - code) - 1;  /* number of real arguments */
  base = code + 1; /* new base set up */
  ci = ci_new(state);  /* now 'enter' new function */
  /* check GIMME */
  //ci->gimme = ;
  ci->code = code;
  ci->base = base;
  ci->top = base;
  //ci->top = base + code->maxregs;
  ci->u.p.savedpc = perl_to_code(*code)->code;  /* starting point */
  state->top = ci->top;
  return;
}

void
perl_postcall(perl_state *state)
{

}

void
stack_init(perl_state *state)
{
  state->stack = malloc(sizeof(perl_scalar)*256); 
  state->stack_size = 256;
  state->top = state->stack; 
  state->stack_end = state->stack + state->stack_size-1;
}

void
perl_run(perl_state *state, perl_code code)
{
  perl_scalar *base;
  perl_callinfo *ci;
  perl_instruction i;
  stack_init(state);
  *(state->top) = code;
  perl_scalar *target = state->top;
  state->top++;
  perl_precall(state, target);
  ci = state->ci; 
 newframe:  /* reentry point when frame changes (call/return) */
  base = ci->u.p.base;
  for (;;) {
    i = *(ci->u.p.savedpc++);
    perl_scalar *ra = RA(i);
    switch (GET_OPCODE(i)) {
      default:
        break;
      case OP_ENTERSUB:
        {
          perl_precall(state, ra);
          ci = state->ci;
          goto newframe;
        }
        break;
      case OP_RETURN:
        {
          perl_postcall(state); 
        }
        break;
    }
  }
}

