#ifndef PERL_CALLINFO_H
#define PERL_CALLINFO_H

typedef struct perl_callinfo {
  perl_scalar *base;
  perl_scalar *code;
  perl_scalar *top;
  int gimme;
  struct perl_callinfo *prev;
  struct perl_callinfo *next;
  union {
    struct {
      perl_scalar *base;
      perl_instruction *savedpc;
    } p;
    struct {
    } c;
  } u;
} perl_callinfo;

#endif
