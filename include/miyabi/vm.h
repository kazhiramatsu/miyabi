#ifndef PERL_VM_H
#define PERL_VM_H

#include "miyabi/perl.h"
#include "miyabi/code.h"
#include "miyabi/value.h"
#include "miyabi/opcode.h"
#include "miyabi/callinfo.h"

void perl_run(perl_state *state, perl_code code);
void perl_precall(perl_state *state, perl_code *code);
void perl_postcall(perl_state *state);

#endif
