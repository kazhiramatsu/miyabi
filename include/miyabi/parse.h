#ifndef PERL_PARSE_H
#define PERL_PARSE_H

#include "miyabi/value.h"
#include "miyabi/opcode.h"
#include "miyabi/node.h"
#include "miyabi/str.h"

enum perl_node_flag {
  NODE_FLAG_PARENS
};

enum {
  XSTATE,
  XREF,
  XTERM,
  XOPERATOR,
  XBLOCK,
	XTERMBLOCK
};

enum lex_strterm {
  LEX_STR_DQUOTE,
};

typedef enum lex_state {
  LEX_NORMAL,
  LEX_KNOWNEXT,
  LEX_SUBSTART,
  LEX_INTERPPUSH,
  LEX_INTERPCONCAT,
  LEX_INTERPSTART,
  LEX_INTERPNORMAL,
  LEX_INTERPEND,
  LEX_INTERPENDMAYBE,
  LEX_PAREN,
  LEX_USESTART,
} lex_state;

typedef enum perl_node_type node_type;

struct perl_parser {
  int saw_infix_sigil;
  _Bool saw_arrow;
  perl_state *state;
  int lex_starts;
  enum lex_state lex_state;
  int in_my;
  int lex_brackets;
  int lex_fakebrack;
  int expect;
  node *comp_unit;
  
  int pending_ident;
  node *curblock;

  struct next_token *nexttoke;

  struct token_s *tokens; /* for debug */

  perl_hash curstash;
  perl_hash defstash;
	perl_scalar curstashname;

  int line;
  char *file;
  int column;
  void *ylval;
  FILE *f;
  int sterm;
  char tokenbuf[1024];
  int tokenlen;

  int line_idx;
  char *bufptr;
  char *oldbufptr;
  char *oldoldbufptr;
  char *bufend;	
  char *linestart;

  char *linestr;
  int bufsize;
  int read_offset;
  int offset;

	perl_scalar subname;
};

node *node_new(perl_parser *p, enum perl_node_type type, uint32_t flags);
void perl_init_node(perl_parser *p, node *base, enum perl_node_type type, int flags);
void node_comp_unit_new(perl_parser *p, node *n);
node *node_append_statement(perl_parser *p, node *first, node *last);
node *node_statementlist_new(perl_parser *p, node *statementlist);
node *node_statement_new(perl_parser *p, char *label, node *expr);
node *node_list_new(perl_parser *p, enum perl_node_type type, node *elem);
node *node_prepend_elem(perl_parser *p, enum perl_node_type type, node *first, node *last);
node *node_append_elem(perl_parser *p, enum perl_node_type type, node *first, node *last);
node *node_unop_new(perl_parser *p, enum perl_node_type type, node *first);
node *node_assign_new(perl_parser *p, node *first, node *last);
node *node_eq_new(perl_parser *p, int eqop, node *first, node *last);
node *node_aelem_new(perl_parser *p, node *first, node *last);
node *node_call_new(perl_parser *p, enum perl_node_type type, node *name, node *args);
node *node_builtin_call_new(perl_parser *p, enum perl_node_type type, int op, node *indirob, node *args);
node *node_unary_call_new(perl_parser *p, enum perl_node_type type, int op);
node *node_sub_new(perl_parser *p, node *startsub, node *subname,
                   node *proto, node *subattrlist, node *subbody);

node *node_use_new(perl_parser *p, int aver, node *floor,
                   node *version, node *id, node *arg);
node *node_package_new(perl_parser *p, node *name);
node *node_block_new(perl_parser *p, node *block);
node *node_logical_new(perl_parser *p, enum perl_node_type type, node *first, node *other);
node *node_while_new(perl_parser *p, enum perl_node_type type, node *cond, node *block);
node *node_until_new(perl_parser *p, enum perl_node_type type, node *cond, node *block);
node *node_block_start(perl_parser *p);
node *node_block_end(perl_parser *p, node *block, node *statementlist);
node *node_for_new(perl_parser *p, node *sv, node *expr, node *block, node *cont);
node *node_if_new(perl_parser *p, node *first, node *true_node, node *false_node);
void perl_in_my(perl_parser *p, int status);
void perl_lex_state(perl_parser *p, enum lex_state lex_state);
node *perl_localize(perl_parser *p, node *n, int lex);
node *perl_sawparens(perl_parser *p, node *n);
node *node_anonlist_new(perl_parser *p, node *node);
node *node_anonhash_new(perl_parser *p, node *node);
node *node_nulllist_new(perl_parser *p);
node *node_glob_ref_new(perl_parser *p, node *name);
node *node_scalar_ref_new(perl_parser *p, node *name);
node *node_array_ref_new(perl_parser *p, node *name);
node *node_hash_ref_new(perl_parser *p, node *name);
perl_variable *variable_new(perl_scalar name, int idx, enum perl_scope scope);
perl_variable *perl_add_my_name(perl_parser *p, perl_scalar name);
void perl_token_dump(perl_parser *p);
void perl_lex_dump(perl_variable *variable, int indent);
void perl_node_dump(node *n, int indent);
perl_parser *perl_parse_file(perl_state *state, char *file);
perl_parser *perl_parser_new(perl_state *state);
perl_scalar perl_add_our_name(perl_parser *p, perl_hash stash, perl_scalar n);
node *node_identifier_new(perl_parser *p, perl_scalar s);
node *node_const_new(perl_parser *p, perl_value value);
node *node_variable_new(perl_parser *p, perl_variable *v);
node *node_qwlist_new(perl_parser *p, perl_scalar v);
char *scan_num(perl_parser *p, char *s);
perl_variable *perl_find_my_name(perl_parser *p, perl_scalar name);
int keylookup(perl_parser *p, char *s, int word);
void force_next(perl_parser *p, int token);
int perl_sublex_start(perl_parser *p, int type);
char *scan_ident(perl_parser *p, char *s, char *dest, ssize_t destlen, int ck_uni);
int parse_word(perl_parser *p, char *s);
int perl_pending_ident(perl_parser *p);
void pushback(perl_parser *p, int c);
_Bool find_our_name(perl_parser *p, char *tokenbuf, int tokenlen);
node *node_method_call_new(perl_parser *p, enum perl_node_type type, node *invovant, node *name, node *args);
char *scan_word(perl_parser *p, char *s, char *tokenbuf, size_t destlen, _Bool allow_package, size_t *slp);


#endif
