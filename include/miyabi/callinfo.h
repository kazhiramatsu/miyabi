#ifndef PERL_CALLINFO_H
#define PERL_CALLINFO_H

typedef struct perl_callinfo {
  perl_scalar *base;
  perl_scalar *func;
  perl_scalar *top;
  perl_instruction *pc;
  int nresults;
  struct perl_callinfo *prev;
  struct perl_callinfo *next;
} perl_callinfo;

#endif
