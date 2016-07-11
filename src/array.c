#include "miyabi/perl.h"

perl_array
perl_array_new(perl_state *state)
{
  struct perl_array *array;
  int i;

  array = malloc(sizeof(struct perl_array));
  array->base.type = PERL_TYPE_ARRAY;
  array->fill = 0;
  array->max = 256;
  array->head = 0;
  array->array = malloc(sizeof(perl_scalar)*(array->max));

  for (i = 0; i < array->max; i++) {
    array->array[i] = perl_undef_new();
  }
  return perl_array_init(array);
}

perl_scalar 
perl_array_shift(perl_state *state, perl_array ary)
{
  perl_scalar result;
  int new_max = 0;
  struct perl_array *array = perl_to_array(ary);
  int target = array->head;

  if (array->fill == 0) {
    return perl_undef_new();
  }
  result = array->array[target];
  array->head++;
  array->fill--;
  return result;
}

perl_scalar 
perl_array_pop(perl_state *state, perl_array ary)
{
  perl_scalar result;
  struct perl_array *array = perl_to_array(ary);
  int target = array->head + array->fill - 1;

  if (array->fill == 0) {
    return perl_undef_new();
  }
  result = array->array[target]; 
  array->fill--;
  return result;
}

perl_scalar *
perl_array_fetch(perl_state *state, perl_array ary, int key, perl_scalar val, _Bool add)
{
  struct perl_array *array = perl_to_array(ary);

  if ((key >= array->max) || (key < 0)) {
    return NULL;
  }

  if (perl_type(array->array[key]) == PERL_TYPE_UNDEF && add) {
    array->array[key] = val;
  }
  return &array->array[key];
}

void
perl_array_push(perl_state *state, perl_array ary, perl_scalar value)
{
  int new_max = 0;
  struct perl_array *array = perl_to_array(ary);
  int target = array->head + array->fill;

  if (target >= array->max) {
    new_max = array->max * 2;
    array->array = realloc(array->array, sizeof(perl_value) * new_max);
    array->max = new_max;
    array->array[target] = value; 
    array->fill++;
    return;
  }
  array->array[target] = value; 
  array->fill++;
}

perl_scalar *
perl_array_store(perl_state *state, perl_array ary, int key, perl_scalar value)
{
  int new_max = 0;
  struct perl_array *array = perl_to_array(ary);

  if (key >= array->max) {
    new_max = array->max * 2;
    array->array = realloc(array->array, sizeof(perl_value) * new_max);
    array->max = new_max;
    array->array[key] = value;
    array->fill++;
    return &array->array[key];
  }
  array->array[key] = value; 
  array->fill++;
  return &array->array[key];
}

int
perl_array_length(perl_state *state, perl_array ary)
{
  struct perl_array *array = perl_to_array(ary);

  return array->fill;
}

void
perl_array_dump(perl_array ary)
{
  struct perl_array *array = perl_to_array(ary);
  perl_scalar scalar;
  int i;

  for (i = 0; i < array->fill; i++) {
    scalar = array->array[i];
    printf("##### array ########\n");
    printf("index = %zd\n", i);
    printf("fill = %zd\n", array->fill);
    printf("max = %zd\n", array->max);
    perl_scalar_dump(scalar);
  }
}

