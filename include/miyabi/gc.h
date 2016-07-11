#ifndef PERL_GC_H
#define PERL_GC_H

typedef struct perl_heap {
  uintptr_t from_start;
  uintptr_t to_start;
  uintptr_t free;
  size_t heap_size;
  uintptr_t *form;
  uintptr_t *to;
} perl_heap;

#endif
