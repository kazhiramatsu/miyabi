#ifndef PERL_VALUE_H
#define PERL_VALUE_H

typedef uint64_t perl_value;

#define PERL_TAG_NAN    UINT64_C(0xFFF0000000000000)
#define PERL_TAG_INT    UINT64_C(0x0001000000000000)
#define PERL_TAG_STR    UINT64_C(0x0002000000000000)
#define PERL_TAG_ARRAY  UINT64_C(0x0003000000000000)
#define PERL_TAG_HASH   UINT64_C(0x0004000000000000)
#define PERL_TAG_CODE   UINT64_C(0x0005000000000000)
#define PERL_TAG_STASH  UINT64_C(0x0006000000000000)
#define PERL_TAG_GLOB   UINT64_C(0x0007000000000000)
#define PERL_TAG_UNDEF  UINT64_C(0x000E000000000000)
#define PERL_PTR_MASK   UINT64_C(0x0000FFFFFFFFFFFF)
#define PERL_TAG_MASK   UINT64_C(0x000F000000000000)

#define perl_type(data) (((uint64_t)(data) & PERL_TAG_NAN) == PERL_TAG_NAN) * (((uint64_t)data & PERL_TAG_MASK) >> 48)

#define perl_check_type(o, T) do {					\
		if (perl_type(o) != T) {						\
			fprintf(stderr, "type = [%llu]\n", perl_type(o));	\
			assert(0 && "Type Error!\n");		\
		}										\
  } while (0)

#define perl_ptr(v) ((void *)((v) & PERL_PTR_MASK))
#define perl_immediate_p(v) (perl_type(v) < PERL_TAG_STR)
#define perl_object_value(v) ((struct perl_object *)(perl_ptr(v)))

#define perl_object_header \
  uint64_t flags;          \
  perl_tag_type tag;

enum perl_zone {
  PERL_ZONE_YOUNG,
};

typedef uint64_t perl_tag_type;
typedef uint64_t perl_scalar;
typedef uint64_t perl_array;
typedef uint64_t perl_hash;
typedef uint64_t perl_undef;
typedef uint64_t perl_glob;
typedef uint64_t perl_code;

enum perl_gc_tag {
  PERL_GC_COPIED
};

typedef struct perl_object {
  perl_object_header
} perl_object;

typedef struct perl_node perl_node;

typedef uint32_t perl_instruction;

enum perl_glob_flags {
  PERL_GLOB_ADD
};

enum perl_glob_type {
  PERL_GLOB_ARRAY,
  PERL_GLOB_CODE
};

#endif
