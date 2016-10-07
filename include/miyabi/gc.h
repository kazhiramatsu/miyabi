#ifndef PERL_GC_H
#define PERL_GC_H

#include "miyabi/perl.h"

enum perl_gc_phase {
	PERL_GC_ROOT_SCAN,
	PERL_GC_MARK,
};

typedef struct perl_gc {
	enum perl_gc_phase phase;
	perl_array mark_stack;
} perl_gc;

typedef struct perl_state perl_state;

perl_gc *perl_gc_new(perl_state *state, size_t heap_size);

#endif
