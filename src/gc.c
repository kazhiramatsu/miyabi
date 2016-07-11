
#include "miyabi/perl.h"

void
gc_copying(perl_state *state)
{
  perl_heap *heap = state->heap;
  perl_object *gc_roots = state->gc_roots;

  heap->free = heap->to_start;
  for (i = 0; i < state->gc_roots->size; i++) {
     state->gc_roots[i] = copy(state->gc_roots[i]);
     swap(heap);
  }
}

void
swap(perl_heap *heap)
{
  uintptr_t to = heap->to_start;
  uintptr_t from = heap->from_start;
  heap->from_start = to;
  heap->to_start = from;
}

perl_object *
copy(perl_heap *heap, perl_object *obj)
{
  if (obj->tag != PERL_GC_COPIED) {
    copy_data(heap->free, obj, obj->size);
    obj->tag = PERL_GC_COPIED;
    obj->forwarding = heap->free;
    heap->free += obj->size; 
    copy_children(heap, obj->forwarding);
  }
  return obj->forwarding;
}

void
copy_children(perl_heap *heap, perl_object *obj)
{
  switch (obj->type) {
  case PERL_TYPE_STR:
    break;
  case PERL_TYPE_ARRAY:
    copy_array(heap, (struct perl_array *)obj);
    break;
  }
}

copy_array(perl_heap *heap, struct perl_array *ary)
{
  int i;

  for (i = 0; i < ary->fill; i++) {
    switch (perl_type(ary->array[i])) {
      case PERL_TYPE_NUM:
      case PERL_TYPE_INT:
      case PERL_TYPE_UNDEF:
        break;
      case PERL_TYPE_ARRAY:
        ary->[i] = copy((perl_object *)perl_to_str(ary->array[i]));
        break;  
    }
  }
}
