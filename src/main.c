
#include "miyabi/perl.h"
#include "miyabi/parse.h"
#include "miyabi/compile.h"

int
main(int argc, char **argv)
{
  perl_state *state;
  perl_parser *p;

  state = perl_new();

  perl_parse_options(state, argc, argv);
  p = perl_parse_file(state, state->filename);

  perl_compile(state, p->program);

  return 0;
}

