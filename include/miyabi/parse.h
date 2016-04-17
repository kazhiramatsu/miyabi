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
  LEX_PAREN,
  LEX_USESTART,
} lex_state;

typedef struct perl_node node;
typedef enum perl_node_type node_type;

struct perl_parser {
  _Bool saw_arrow;
  perl_state *state;
  int lex_starts;
  enum lex_state lex_state;
  int in_my;
  int lex_brackets;
  int expect;
  int current;
  node *program;
  
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

node *perl_new_node(perl_parser *p, enum perl_node_type type, uint32_t flags);
void perl_init_node(perl_parser *p, node *base, enum perl_node_type type, int flags);
node *perl_new_statementlist(perl_parser *p, node *statement);
node *perl_append_list(perl_parser *p, node *first, node *last);
node *perl_new_list(perl_parser *p, enum perl_node_type type, node *elem);
node *perl_convert(perl_parser *p, enum perl_node_type type, node *list);
node *perl_prepend_elem(perl_parser *p, enum perl_node_type type, node *first, node *last);
node *perl_append_elem(perl_parser *p, enum perl_node_type type, node *first, node *last);
node *perl_new_unop(perl_parser *p, enum perl_node_type type, node *first);
node *perl_new_assign(perl_parser *p, node *first, node *last);
node *perl_new_aelem(perl_parser *p, node *first, node *last);
node *perl_new_call(perl_parser *p, enum perl_node_type type, node *name, node *args);
node *perl_new_builtin_call(perl_parser *p, enum perl_node_type type, int op, node *indirob, node *args);
node *perl_new_unary_call(perl_parser *p, enum perl_node_type type, int op);
node *perl_new_sub_node(perl_parser *p, node *startsub, node *subname,
                        node *proto, node *subattrlist, node *subbody);

node *perl_new_use(perl_parser *p, int aver, node *floor,
                   node *version, node *id, node *arg);
node *perl_new_package(perl_parser *p, node *name);
static int yyparse(perl_parser *p);
static int yylex(void *lval, perl_parser *p);
static void yyerror(perl_parser *p, const char *s);
static void perl_warner(perl_parser *p, const char *format, ...);
static void yywarn(perl_parser *p, const char *s);
static void yywarning(perl_parser *p, const char *s);
static int perl_yylex(perl_parser *p);
static void perl_warner(perl_parser *p, const char *format, ...);
static void yywarn(perl_parser *p, const char *s);
static void yyerror(perl_parser *p, const char *s);
node *perl_new_block(perl_parser *p, node *block);
node *perl_new_logical(perl_parser *p, enum perl_node_type type, node *first, node *other);
node *perl_new_while(perl_parser *p, enum perl_node_type type, node *cond, node *block);
node *perl_new_until(perl_parser *p, enum perl_node_type type, node *cond, node *block);
node *perl_block_start(perl_parser *p);
node *perl_block_end(perl_parser *p, node *block, node *statementlist);
node *perl_new_for(perl_parser *p, node *sv, node *expr, node *block, node *cont);
node *perl_new_if(perl_parser *p, node *first, node *true_node, node *false_node);
void perl_in_my(perl_parser *p, int status);
void perl_lex_state(perl_parser *p, enum lex_state lex_state);
node *perl_localize(perl_parser *p, node *n, int lex);
node *perl_sawparens(perl_parser *p, node *n);
node *perl_new_anonlist(perl_parser *p, node *node);
node *perl_new_anonhash(perl_parser *p, node *node);
node *perl_new_nulllist(perl_parser *p);
node *perl_new_gvref(perl_parser *p, node *name);
node *perl_new_svref(perl_parser *p, node *name);
node *perl_new_avref(perl_parser *p, node *name);
node *perl_new_hvref(perl_parser *p, node *name);
perl_variable *perl_new_variable(perl_scalar name, int idx, enum perl_scope scope);
perl_variable *perl_add_my_name(perl_parser *p, perl_scalar name);
void perl_new_ast(perl_parser *p, node *n);
void perl_token_dump(perl_parser *p);
void perl_lex_dump(perl_variable *variable, int indent);
void perl_node_dump(node *n, int indent);
perl_parser *perl_parse_file(perl_state *state, char *file);
perl_parser *perl_parser_new(perl_state *state);
perl_scalar perl_add_our_name(perl_parser *p, perl_hash stash, perl_scalar n);
node *perl_new_sym(perl_parser *p, perl_scalar s);
node *perl_new_const(perl_parser *p, perl_value value);
node *perl_new_variable_node(perl_parser *p, perl_variable *v);
node *perl_new_qwlist(perl_parser *p, perl_scalar v);
char *scan_num(perl_parser *p, char *s);
perl_variable *perl_find_my_name(perl_parser *p, perl_scalar name);
int keylookup(perl_parser *p, char *s, int word);
void force_next(perl_parser *p, int token);
int perl_sublex_start(perl_parser *p, int type);
int parse_word(perl_parser *p, char *s);
char *scan_ident(perl_parser *p, char *s);
int perl_pending_ident(perl_parser *p);
void next(perl_parser *p);
void pushback_n(perl_parser *p, char *buf, int n);
void pushback(perl_parser *p, int c);
_Bool peek(perl_parser *p, char *str, int n);
_Bool find_our_name(perl_parser *p, char *tokenbuf, int tokenlen);
node *perl_new_method_call(perl_parser *p, enum perl_node_type type, node *invovant, node *name, node *args);
void perl_new_program(perl_parser *p, node *n);
char *scan_word(perl_parser *p, char *s, char *tokenbuf, size_t destlen, _Bool allow_package, size_t *slp);


#endif
