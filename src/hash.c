#include "miyabi/perl.h"
#include "miyabi/str.h"
#include "miyabi/hash.h"

perl_hash
perl_hash_new(perl_state *state)
{
  struct perl_hash *table;
  int i;

  table = malloc(sizeof(struct perl_hash));
  table->base.header.type = PERL_TYPE_HASH;
  table->keys = 0;
  table->fill = 0;
  table->max = 256;
  table->iter_idx = 0;
  table->iter = NULL;
  table->array = malloc(sizeof(perl_hash_entry*)*(table->max));

  for (i = 0; i < table->max; i++) {
    table->array[i] = NULL;
  }
  return perl_hash_init(table);
}

int
perl_hash_code(perl_scalar scalar)
{
  int i = 0;
	struct perl_str *str = perl_to_str(scalar);
  int h = str->hash; 

  if (h == 0) {
    int off = 0;
    char *val = str->str;
    int len = str->fill;

    for (int i = 0; i < len; i++) {
      h = 31*h + val[off++];
    }
    str->hash = h;
  }
  return h;
}

perl_scalar *
perl_hash_store(perl_state *state, perl_hash hash, perl_scalar key, perl_scalar value)
{
  int h, index, found = 0;
  perl_hash_entry *iter, *prev, *entry;
	struct perl_hash *table = perl_to_hash(hash);

  h = perl_hash_code(key);
  index = h & table->max-1;
  if (table->array[index] == NULL) {
    entry = perl_hash_entry_new(state, key, value);
    table->array[index] = entry;
    table->keys++;
    table->fill++;
    return &table->array[index]->value;
  }
  entry = table->array[index];
  for (iter = entry; iter; prev = iter, iter = iter->next) {
    if (strcmp(perl_to_str(key)->str, perl_to_str(iter->key)->str) == 0) {
      iter->value = value;
      return &iter->value;
    }
  }
  if (table->fill / table->keys) {

  }
  prev->next = perl_hash_entry_new(state, key, value);
  table->keys++;
  return &prev->next->value;
}

perl_scalar *
perl_hash_fetch(perl_state *state, perl_hash hash, perl_scalar key, perl_scalar value, _Bool add)
{
  int h, index, found = 0;
  perl_hash_entry *iter, *prev, *entry;
	struct perl_hash *table = perl_to_hash(hash);

  h = perl_hash_code(key);
  index = h & table->max-1;
  if (table->array[index] == NULL) {
    if (add) {
      entry = perl_hash_entry_new(state, key, value);
      table->array[index] = entry;
      table->keys++;
      table->fill++;
      return &table->array[index]->value;
    }
    return NULL;
  }
  entry = table->array[index];
  for (iter = entry; iter; prev = iter, iter = iter->next) {
    if (strcmp(perl_to_str(key)->str, perl_to_str(iter->key)->str) == 0) {
      return &iter->value;
    }
  }
  if (table->fill / table->keys) {

  }
  if (add) {
    prev->next = perl_hash_entry_new(state, key, value);
    table->fill++;
    return &prev->next->value;
  }
  return NULL;
}

void
perl_hash_iter_init(perl_hash hash)
{
	struct perl_hash *table = perl_to_hash(hash);
  table->iter_idx = -1;
  table->iter = NULL;
}

perl_hash_entry *
perl_hash_iter_next(perl_hash hash)
{
	struct perl_hash *table = perl_to_hash(hash);
  perl_hash_entry *entry;

  entry = table->iter;
  if (entry) {
    entry = entry->next;
  }
  while (!entry) {
    ++table->iter_idx;
    if (table->iter_idx > table->max) {
      table->iter_idx = -1;
      break;
    }
    entry = table->array[table->iter_idx];
  }
  table->iter = entry;
  return entry;
}

perl_hash_entry *
perl_hash_entry_new(perl_state *state, perl_scalar key, perl_scalar value)
{
  perl_hash_entry *entry;

  entry = malloc(sizeof(perl_hash_entry));
  entry->key = key;
  entry->value = value;
  entry->next = NULL;
  return entry;
}

void
perl_hash_rehash(perl_state *state, perl_hash hash)
{
  perl_hash_entry *entry, *iter, *prev;
	struct perl_hash *table = perl_to_hash(hash);
  size_t new_max = table->max * 2;
  perl_hash_entry **new_array = NULL;

  new_array = realloc(new_array, sizeof(perl_hash_entry*)*new_max);

  for (size_t i = 0; i < table->max; ++i) {
    if (table->array[i] == NULL) {
      continue; 
    }
    entry = table->array[i];
    for (iter = entry, prev = iter; iter; prev = iter, iter = iter->next) {
      perl_store_entry(new_array, new_max, iter);
    }
  }
  free(table->array);
  table->array = NULL;
  table->array = new_array;
  table->fill = table->max;
  table->max = new_max;
}

perl_scalar *
perl_store_entry(perl_hash_entry **array, int new_max, perl_hash_entry *new)
{
  size_t index = perl_to_str(new->key)->hash & new_max-1;
  perl_hash_entry *iter, *entry, *prev;

  if (array[index] == NULL) {
    array[index] = new;
    return &array[index]->value;
  } else {
    entry = array[index];
    for (iter = entry, prev = iter; iter; prev = iter, iter = iter->next);
    prev->next = new;
    return &prev->next->value;
  }
}

void
perl_hash_dump(perl_hash hash)
{
	struct perl_hash *table = perl_to_hash(hash);
  perl_hash_entry *entry, *iter;
  int i;

  for (i = 0; i < table->max; i++) {
    entry = table->array[i];
    for (iter = entry; iter != NULL; iter = iter->next) {
      perl_scalar_dump(iter->key);
      printf("\n");
      perl_scalar_dump(iter->value);
    }
  }
}

