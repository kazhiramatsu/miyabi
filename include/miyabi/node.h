#ifndef PERL_NODE_H
#define PERL_NODE_H

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/str.h"

enum perl_node_type {
  NODE_PROGRAM,
  NODE_NUMBER,
  NODE_STATEMENTLIST,
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

typedef struct perl_program_node {
  perl_node base;
  perl_node *program;
} perl_program_node;

typedef struct perl_if_node {
  perl_node base;
  perl_node *first;
  perl_node *other;
  perl_node *next;
} perl_if_node;

typedef struct perl_logical_node {
  perl_node base;
  perl_node *first;
  perl_node *other;
  perl_node *next;
} perl_logical_node;

typedef struct perl_for_node {
  perl_node base;
  perl_node *sv;
  perl_node *expr;
  perl_node *block;
  perl_node *cont;
} perl_for_node;

typedef struct perl_method_call_node {
  perl_node base;
  perl_node *invocant;
  perl_node *name;
  perl_node *args;
} perl_method_call_node;

typedef struct perl_list_node {
  perl_node base;
  int op;
  perl_node *elem;
  perl_node *next;
  perl_node *last;
} perl_list_node;

typedef struct perl_binop_node {
  perl_node base;
  int op;
  perl_node *first;
  perl_node *last;
} perl_binop_node;

typedef struct perl_unop_node {
  perl_node base;
  int op;
  perl_node *first;
} perl_unop_node;

typedef struct perl_uniop_node {
  perl_node base;
  int op;
} perl_uniop_node;

typedef struct perl_statementlist_node {
  perl_node base;
  perl_node *statement;
  perl_node *next;
  perl_node *last;
} perl_statementlist_node;

typedef struct perl_statement_node {
  perl_node base;
  perl_scalar label;
  perl_node *statement;
} perl_statement_node;

typedef struct perl_sub_node {
  perl_node base;
  perl_hash stash;
  perl_scalar stashname;
  perl_node *startsub;
  perl_node *subname;
  perl_node *proto;
  perl_node *subattrlist;
  perl_node *subbody;
} perl_sub_node;

typedef struct perl_package_node {
  perl_node base;
  perl_node *name;
} perl_package_node;

typedef struct perl_call_node {
  perl_node base;
  int op;
  perl_node *name;
  perl_node *indirob;
  perl_node *args;
} perl_call_node;

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

typedef struct perl_block_node {
  perl_node base;
  perl_node *statementlist;
  perl_variable *variable;
  perl_const *k;
  perl_node *outer;
} perl_block_node;

typedef struct perl_variable_node {
  perl_node base;
  perl_variable *variable;
  perl_node *myblock;
} perl_variable_node;

typedef struct perl_value_node {
  perl_node base;
  perl_scalar value;
} perl_value_node;

typedef struct perl_use_node {
  perl_node base;
  perl_node *floor;
  perl_node *version;
  perl_node *id;
  perl_node *arg;
} perl_use_node;

typedef struct perl_loop_node {
  perl_node base;
  perl_node *cond;
  perl_node *block;
} perl_loop_node;

typedef struct perl_sym_node {
  perl_node base;
  perl_scalar sym;
} perl_sym_node;

typedef struct perl_const_node {
  perl_node base;
  perl_scalar value;
} perl_const_node;

typedef struct keyword_s {
  char *key_name;
  int key_id;
  _Bool is_core;
} keyword;

perl_statementlist_node *to_statementlist_node(perl_node *n);
perl_list_node *to_list_node(perl_node *n);
perl_value_node *to_value_node(perl_node *n);
perl_variable_node *to_variable_node(perl_node *n);
perl_call_node *to_call_node(perl_node *n);
perl_package_node *to_package_node(perl_node *n);
perl_sub_node *to_sub_node(perl_node *n);
perl_binop_node *to_binop_node(perl_node *n);
perl_block_node *to_block_node(perl_node *n);
perl_unop_node *to_unop_node(perl_node *n);
perl_logical_node *to_logical_node(perl_node *n);
perl_for_node *to_for_node(perl_node *n);
perl_use_node *to_use_node(perl_node *n);
perl_const_node *to_const_node(perl_node *n);
perl_sym_node *to_sym_node(perl_node *n);
perl_program_node *to_program_node(perl_node *n);

#endif
