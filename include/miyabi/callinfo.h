#ifndef PERL_CALLINFO_H
#define PERL_CALLINFO_H

typedef struct perl_callinfo {
  perl_value *base;
  perl_value *func;
  perl_value *top;
  perl_instruction *pc;
  int nresults;
  struct perl_callinfo *prev;
  struct perl_callinfo *next;
} perl_callinfo;

#endif
