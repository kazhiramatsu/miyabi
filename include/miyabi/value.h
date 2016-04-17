#ifndef PERL_VALUE_H
#define PERL_VALUE_H

typedef uint64_t perl_value;

#define PERL_TAG_NAN    UINT64_C(0xFFF0000000000000)
#define PERL_TAG_INT    UINT64_C(0x0001000000000000)
#define PERL_VALUE_MASK UINT64_C(0x0000FFFFFFFFFFFF)
#define PERL_TAG_STR    UINT64_C(0x0002000000000000)
#define PERL_TAG_ARRAY  UINT64_C(0x0003000000000000)
#define PERL_TAG_HASH   UINT64_C(0x0004000000000000)
#define PERL_TAG_CODE   UINT64_C(0x0005000000000000)
#define PERL_TAG_STASH  UINT64_C(0x0006000000000000)
#define PERL_TAG_GLOB   UINT64_C(0x0007000000000000)
#define PERL_TAG_UNDEF  UINT64_C(0x000E000000000000)
#define PERL_TYPE_MASK  UINT64_C(0x000F000000000000)

#define perl_type(data) (((uint64_t)(data) & PERL_TAG_NAN) == PERL_TAG_NAN) * (((uint64_t)data & PERL_TYPE_MASK) >> 48)

#define perl_check_type(o, T) do {					\
		if (perl_type(o) != T) {						\
			fprintf(stderr, "type = [%llu]\n", perl_type(o));	\
			assert(0 && "Type Error!\n");		\
		}										\
  } while (0)

enum perl_zone {
  PERL_ZONE_YOUNG,
};

enum perl_type {
  PERL_TYPE_NUM,
  PERL_TYPE_INT,
  PERL_TYPE_STR,
  PERL_TYPE_ARRAY,
  PERL_TYPE_HASH,
  PERL_TYPE_CODE,
  PERL_TYPE_GLOB,
  PERL_TYPE_UNDEF,
};

typedef uint64_t perl_tag_type;
typedef uint64_t perl_scalar;
typedef uint64_t perl_array;
typedef uint64_t perl_hash;
typedef uint64_t perl_undef;
typedef uint64_t perl_glob;
typedef uint64_t perl_code;

typedef struct perl_object_header perl_object_header;

struct perl_object_header {
  unsigned int type:8;
  unsigned int zone:2;
  unsigned int age:4;
  unsigned int forwarded:1;
  unsigned int remembered:1;
  unsigned int marked:2;
  unsigned int require_cleanup:1;
  unsigned int refs_are_weak:1;
  perl_object_header *next;
};

typedef struct perl_object {
  perl_object_header header;
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
