#ifndef PERL_H
#define PERL_H


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "miyabi/opcode.h"
#include "miyabi/value.h"
#include "miyabi/callinfo.h"
#include "miyabi/state.h"
#include "miyabi/str.h"
#include "miyabi/array.h"
#include "miyabi/code.h"
#include "miyabi/hash.h"
#include "miyabi/glob.h"

#define GC_DEBUG
#include <gc.h>
#define malloc(n) GC_MALLOC(n)
#define calloc(m,n) GC_MALLOC((m)*(n))
#define free(p) GC_FREE(p)
#define realloc(p,n) GC_REALLOC((p),(n))
#define CHECK_LEAKS() GC_gcollect()

#define identifier_start(c) (isalpha(c) || (c) == '_')
#define identifier(c) (isalnum(c) || (c) == '_')
#define PERL_IDENT_MAX 252

#define strNE(s1,s2) (strcmp(s1,s2))
#define strEQ(s1,s2) (!strcmp(s1,s2))
#define strLT(s1,s2) (strcmp(s1,s2) < 0)
#define strLE(s1,s2) (strcmp(s1,s2) <= 0)
#define strGT(s1,s2) (strcmp(s1,s2) > 0)
#define strGE(s1,s2) (strcmp(s1,s2) >= 0)
#define strnNE(s1,s2,l) (strncmp(s1,s2,l))
#define strnEQ(s1,s2,l) (!strncmp(s1,s2,l))
#define cast(t, exp)	((t)(exp))

enum perl_options {
	PERL_OPTION_AST = 1,
	PERL_OPTION_TOKEN,
	PERL_OPTION_OPCODE,
	PERL_OPTION_VERBOSE,
};

struct perl_vm_heaps;

typedef struct perl_parser perl_parser;

perl_state *perl_new(void);
void *perl_saveptr(void *s, unsigned long len);
void perl_warn(perl_state *state, const char *format, ...);
void perl_croak(perl_state *state, const char *format, ...);
void perl_parse_options(perl_state *state, int argc, char **argv);
perl_scalar perl_str_init(struct perl_str *str);
perl_scalar perl_num_init(double val);
perl_scalar perl_int_init(uint64_t val);
perl_array perl_array_init(struct perl_array *array);
perl_hash perl_hash_init(struct perl_hash *hash);
perl_undef perl_undef_new(void);
perl_code perl_code_init(struct perl_code *code);
perl_glob perl_glob_init(struct perl_glob *glob);
_Bool perl_undef_p(perl_scalar val);
double perl_to_num(perl_scalar val);
uint64_t perl_to_int(perl_scalar val);
struct perl_str *perl_to_str(perl_scalar val);
struct perl_array *perl_to_array(perl_array val);
struct perl_hash *perl_to_hash(perl_hash val);
struct perl_code *perl_to_code(perl_code val);
struct perl_glob *perl_to_glob(perl_glob val);
void perl_scalar_dump(perl_scalar v);


#endif
