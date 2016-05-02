#include "miyabi/node.h"

node_statementlist *
to_node_statementlist(perl_node *n)
{
  return (node_statementlist *)n;
}

node_statement *
to_node_statement(perl_node *n)
{
  return (node_statement *)n;
}

node_list *
to_node_list(perl_node *n)
{
  return (node_list *)n;
}

node_value *
to_node_value(perl_node *n)
{
  return (node_value *)n;
}

node_variable *
to_node_variable(perl_node *n)
{
  return (node_variable *)n;
}

node_call *
to_node_call(perl_node *n)
{
  return (node_call *)n;
}

node_package *
to_node_package(perl_node *n)
{
  return (node_package *)n;
}

node_sub *
to_node_sub(perl_node *n)
{
  return (node_sub *)n;
}

node_binop *
to_node_binop(perl_node *n)
{
  return (node_binop *)n;
}

node_block *
to_node_block(perl_node *n)
{
  return (node_block *)n;
}

node_unop *
to_node_unop(perl_node *n)
{
  return (node_unop *)n;
}

node_logical *
to_node_logical(perl_node *n)
{
  return (node_logical *)n;
}

node_for *
to_node_for(perl_node *n)
{
  return (node_for *)n;
}

node_use *
to_node_use(perl_node *n)
{
  return (node_use *)n;
}

node_const *
to_node_const(perl_node *n)
{
  return (node_const *)n;
}

node_sym *
to_node_sym(perl_node *n)
{
  return (node_sym *)n;
}

node_comp_unit *
to_node_comp_unit(perl_node *n)
{
  return (node_comp_unit *)n;
}

