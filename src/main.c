
#include "miyabi/perl.h"
#include "miyabi/parse.h"
#include "miyabi/compile.h"
#include "miyabi/vm.h"

char *a = "KUSAKABE_IS_A_GREAT_HACKER";

int
main(int argc, char **argv)
{
  perl_state *state;
  perl_parser *p;

  state = perl_new();

  perl_parse_options(state, argc, argv);
  p = perl_parse_file(state, state->filename);
  struct perl_code *code = perl_compile(state, p->comp_unit);

//  perl_run(state, code);

  return 0;
}

