
#include "miyabi/perl.h"

void
perl_parse_options(perl_state *state, int argc, char **argv)
{
  state->argc = argc;
  state->argv = argv;
  int i;

  if (argc <= 1) {
    fprintf(stderr, "Script file is missing\n");
    exit(EXIT_FAILURE);
  }

  state->options = 0;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--verbose") == 0) {
      state->options |= PERL_OPTION_VERBOSE;
    } else if (strcmp(argv[i], "--ast") == 0) {
      state->options |= PERL_OPTION_AST;
    } else if (strcmp(argv[i], "--token") == 0) {
      state->options |= PERL_OPTION_TOKEN;
    } else if (strcmp(argv[i], "--opcode") == 0) {
      state->options |= PERL_OPTION_OPCODE;
		}
  }

  state->filename = argv[i-1];
}
