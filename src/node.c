#include "miyabi/node.h"

perl_statementlist_node *
to_statementlist_node(perl_node *n)
{
  return (perl_statementlist_node *)n;
}

perl_list_node *
to_list_node(perl_node *n)
{
  return (perl_list_node *)n;
}

perl_value_node *
to_value_node(perl_node *n)
{
  return (perl_value_node *)n;
}

perl_variable_node *
to_variable_node(perl_node *n)
{
  return (perl_variable_node *)n;
}

perl_call_node *
to_call_node(perl_node *n)
{
  return (perl_call_node *)n;
}

perl_package_node *
to_package_node(perl_node *n)
{
  return (perl_package_node *)n;
}

perl_sub_node *
to_sub_node(perl_node *n)
{
  return (perl_sub_node *)n;
}

perl_binop_node *
to_binop_node(perl_node *n)
{
  return (perl_binop_node *)n;
}

perl_block_node *
to_block_node(perl_node *n)
{
  return (perl_block_node *)n;
}

perl_unop_node *
to_unop_node(perl_node *n)
{
  return (perl_unop_node *)n;
}

perl_logical_node *
to_logical_node(perl_node *n)
{
  return (perl_logical_node *)n;
}

perl_for_node *
to_for_node(perl_node *n)
{
  return (perl_for_node *)n;
}

perl_use_node *
to_use_node(perl_node *n)
{
  return (perl_use_node *)n;
}

perl_const_node *
to_const_node(perl_node *n)
{
  return (perl_const_node *)n;
}

perl_sym_node *
to_sym_node(perl_node *n)
{
  return (perl_sym_node *)n;
}

perl_program_node *
to_program_node(perl_node *n)
{
  return (perl_program_node *)n;
}

