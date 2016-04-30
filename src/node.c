#include "miyabi/node.h"

node_statementlist *
to_statementlist_node(perl_node *n)
{
  return (node_statementlist *)n;
}

node_list *
to_list_node(perl_node *n)
{
  return (node_list *)n;
}

node_value *
to_value_node(perl_node *n)
{
  return (node_value *)n;
}

node_variable *
to_variable_node(perl_node *n)
{
  return (node_variable *)n;
}

node_call *
to_call_node(perl_node *n)
{
  return (node_call *)n;
}

node_package *
to_package_node(perl_node *n)
{
  return (node_package *)n;
}

node_sub *
to_sub_node(perl_node *n)
{
  return (node_sub *)n;
}

node_binop *
to_binop_node(perl_node *n)
{
  return (node_binop *)n;
}

node_block *
to_block_node(perl_node *n)
{
  return (node_block *)n;
}

node_unop *
to_unop_node(perl_node *n)
{
  return (node_unop *)n;
}

node_logical *
to_logical_node(perl_node *n)
{
  return (node_logical *)n;
}

node_for *
to_for_node(perl_node *n)
{
  return (node_for *)n;
}

node_use *
to_use_node(perl_node *n)
{
  return (node_use *)n;
}

node_const *
to_const_node(perl_node *n)
{
  return (node_const *)n;
}

node_sym *
to_sym_node(perl_node *n)
{
  return (node_sym *)n;
}

node_program *
to_node_program(perl_node *n)
{
  return (node_program *)n;
}

