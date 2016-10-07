#ifndef PERL_HASH_H
#define PERL_HASH_H

#include "miyabi/perl.h"
#include "miyabi/value.h"

typedef struct perl_hash_entry {
  perl_scalar key;
  perl_scalar value;
  struct perl_hash_entry *next;
} perl_hash_entry;

struct perl_hash {
  perl_object_header
  int keys;
  int fill;
  int max;
  int iter_idx;
  perl_hash_entry *iter;
  perl_hash_entry **array;
};

perl_hash perl_hash_new(perl_state *state);
int perl_hash_code(perl_scalar str);
perl_scalar *perl_hash_store(perl_state *state, perl_hash hash, perl_scalar key, perl_scalar value);
perl_scalar *perl_hash_fetch(perl_state *state, perl_hash hash, perl_scalar key, perl_scalar value, _Bool add);
void perl_hash_iter_init(perl_hash hash);
perl_hash_entry *perl_hash_iter_next(perl_hash hash);
void perl_hash_dump(perl_hash table);
perl_hash_entry *perl_hash_entry_new(perl_state *state, perl_scalar key, perl_value value);
perl_value *perl_store_entry(perl_hash_entry **array, int new_max, perl_hash_entry *new);
void perl_hash_rehash(perl_state *state, perl_hash hash);

#endif
