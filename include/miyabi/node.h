#ifndef PERL_NODE_H
#define PERL_NODE_H

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/str.h"

typedef struct perl_node node;

enum perl_node_type {
  NODE_PROGRAM,
  NODE_NUMBER,
  NODE_STATEMENTLIST,
  NODE_STATEMENT,
  NODE_SASSIGN,
  NODE_AASSIGN,
  NODE_STR,
  NODE_ANYVAR,
  NODE_CONST,
  NODE_SYM,
  NODE_SCALARVAR,
  NODE_ARRAYVAR,
  NODE_HASHVAR,
  NODE_ENTERSUB,
  NODE_SUB,
  NODE_METHOD,
  NODE_BLOCK,
  NODE_PACKAGE,
  NODE_CALL,
  NODE_NEGATE,
  NODE_LIST,
  NODE_ANONLIST,
  NODE_ANONHASH,
  NODE_USE,
  NODE_IF,
  NODE_AND,
  NODE_OR,
  NODE_UNLESS,
  NODE_FOR,
  NODE_AELEM,
  NODE_QWLIST,
  NODE_METHOD_CALL,
  NODE_WHILE,
  NODE_UNTIL,
  NODE_GV,
  NODE_EQ,
  NODE_NE,
  NODE_SEQ,
  NODE_SNE,
};

struct perl_node {
  enum perl_node_type type;
  uint32_t flags;
  size_t pos;
  size_t line;
};

typedef struct node_program {
  perl_node base;
  perl_node *program;
} node_program;

typedef struct node_if {
  perl_node base;
  perl_node *first;
  perl_node *other;
  perl_node *next;
} node_if;

typedef struct node_logical {
  perl_node base;
  perl_node *first;
  perl_node *other;
  perl_node *next;
} node_logical;

typedef struct node_for {
  perl_node base;
  perl_node *sv;
  perl_node *expr;
  perl_node *block;
  perl_node *cont;
} node_for;

typedef struct node_method_call {
  perl_node base;
  perl_node *invocant;
  perl_node *name;
  perl_node *args;
} node_method_call;

typedef struct node_list {
  perl_node base;
  int op;
  perl_node *elem;
  perl_node *next;
  perl_node *last;
} node_list;

typedef struct node_binop {
  perl_node base;
  int op;
  perl_node *first;
  perl_node *last;
} node_binop;

typedef struct node_unop {
  perl_node base;
  int op;
  perl_node *first;
} node_unop;

typedef struct node_uniop {
  perl_node base;
  int op;
} node_uniop;

typedef struct node_statementlist {
  perl_node base;
  perl_node *statement;
  perl_node *next;
  perl_node *last;
} node_statementlist;

typedef struct node_statement {
  perl_node base;
  perl_scalar label;
  perl_node *statement;
} node_statement;

typedef struct node_sub {
  perl_node base;
  perl_hash stash;
  perl_scalar stashname;
  perl_node *startsub;
  perl_node *subname;
  perl_node *proto;
  perl_node *subattrlist;
  perl_node *subbody;
} node_sub;

typedef struct node_package {
  perl_node base;
  perl_node *name;
} node_package;

typedef struct node_call {
  perl_node base;
  int op;
  perl_node *name;
  perl_node *indirob;
  perl_node *args;
} node_call;

enum perl_scope {
  PERL_SCOPE_OUR,
  PERL_SCOPE_MY,
};

typedef struct perl_variable {
  perl_scalar name;
  enum perl_scope scope;
  int idx;
  char *file;
  int line;
  struct perl_variable *next;
} perl_variable;

typedef struct perl_const {
  perl_scalar value;
  int idx;
  struct perl_const *first;
  struct perl_const *last;
} perl_const;

typedef struct node_block {
  perl_node base;
  perl_node *statementlist;
  perl_variable *variable;
  perl_const *k;
  perl_node *outer;
} node_block;

typedef struct node_variable {
  perl_node base;
  perl_variable *variable;
  perl_node *myblock;
} node_variable;

typedef struct node_value {
  perl_node base;
  perl_scalar value;
} node_value;

typedef struct node_use {
  perl_node base;
  perl_node *floor;
  perl_node *version;
  perl_node *id;
  perl_node *arg;
} node_use;

typedef struct node_loop {
  perl_node base;
  perl_node *cond;
  perl_node *block;
} node_loop;

typedef struct node_sym {
  perl_node base;
  perl_scalar sym;
} node_sym;

typedef struct node_const {
  perl_node base;
  perl_scalar value;
} node_const;

typedef struct keyword_s {
  char *key_name;
  int key_id;
  _Bool is_core;
} keyword;

void node_program_new(perl_parser *p, node *n);
node_statementlist *to_node_statementlist(node *n);
node_list *to_node_list(perl_node *n);
node_value *to_node_value(perl_node *n);
node_variable *to_node_variable(perl_node *n);
node_call *to_node_call(perl_node *n);
node_package *to_node_package(perl_node *n);
node_sub *to_node_sub(perl_node *n);
node_binop *to_node_binop(perl_node *n);
node_block *to_node_block(perl_node *n);
node_unop *to_node_unop(perl_node *n);
node_logical *to_node_logical(perl_node *n);
node_for *to_node_for(perl_node *n);
node_use *to_node_use(perl_node *n);
node_const *to_node_const(perl_node *n);
node_sym *to_node_sym(perl_node *n);
node_program *to_node_program(perl_node *n);

#endif
