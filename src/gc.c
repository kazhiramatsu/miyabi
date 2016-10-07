
#include "miyabi/perl.h"

perl_gc *
perl_gc_new(perl_state *state, size_t heap_size)
{
	perl_gc *gc;

	gc = malloc(sizeof(perl_gc));
  gc->phase = PERL_GC_ROOT_SCAN;
  gc->mark_stack = perl_array_new(state); 
	return gc;
}

void
scan_stack(perl_state *state)
{
  int i;
  size_t size = state->stack_end - state->stack;
  perl_scalar *stack = state->stack;

  for (i = 0; i < size; i++) {
    if (perl_immediate_p(stack[i])) {
      continue;
    }
    struct perl_object *obj = perl_object_value(stack[i]);
    if (obj->flags & PERL_GC_MARK) {
      obj->flags |= PERL_GC_MARK;
    } 
    perl_array_push(state, state->gc->mark_stack, stack[i]);
  }
}

void
root_scan_phase(perl_state *state)
{
  scan_stack(state);
  state->gc->phase = PERL_GC_MARK;
}

void
perl_incremental_gc(perl_state *state)
{
  switch (state->gc->phase) {
  case PERL_GC_ROOT_SCAN:
    root_scan_phase(state);
    break;
  case PERL_GC_MARK:
    root_scan_phase(state);
    break;
  }
}

