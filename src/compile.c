#include "miyabi/compile.h"

struct compiler_unit {
  compiler_unit *prev;
  perl_instruction *code;
  int fill;
  int max;
  int reg;
  perl_array consts;
  perl_array syms;
  perl_hash comp_stash;
};

static
int
reserve_reg(perl_node *n)
{
  int size = 0;
  perl_variable *v = NULL;

  if (!n) return size;

  for (v = to_node_block(n)->variable; v; v = v->next) size++;
  return size;
}

static compiler_unit *
compiler_unit_new(perl_compiler *c, compiler_unit *prev, perl_node *n)
{
  compiler_unit *u;

  u = malloc(sizeof(compiler_unit));
  u->prev = prev;
  u->code = malloc(sizeof(perl_instruction)*256);
  u->fill = 0;
  u->max = 256;
  u->consts = perl_array_new(c->state);
  u->reg = reserve_reg(n);
  u->comp_stash = c->curstash;

  return u;
}

static perl_code
build_code(perl_compiler *c, compiler_unit *u)
{
  perl_code code;

  code = perl_code_new(c->state);
  perl_to_code(code)->size = u->fill;
  perl_to_code(code)->code = realloc(u->code, sizeof(perl_instruction) * u->fill);
  perl_to_code(code)->size = u->fill;

	return code;
}

static perl_code
compiler_unit_finish(perl_compiler *c, compiler_unit *u)
{
  perl_code code = build_code(c, u);
  return code;
}

void
peep(perl_compiler *c, compiler_unit *u, perl_instruction cur, int reg)
{
  int curop = GET_OPCODE(cur);
  perl_instruction prev = u->code[u->fill-1];
  int prevop = GET_OPCODE(prev);

  switch (curop) {
    case OP_SASSIGN:
      {
        switch (prevop) {
          case OP_CONST:
            {
              if (GETARG_B(cur) == GETARG_A(prev)) {
                u->code[u->fill-1] = CREATE_ABC(OP_CONST, GETARG_A(cur), GETARG_B(prev), 0);
              }
            }
        }
      }
  }
}

int
add_const(perl_compiler *c, compiler_unit *u, perl_node *n)
{
  node_const *k = to_node_const(n);

  int idx = perl_array_length(c->state, u->consts);
  perl_array_push(c->state, u->consts, k->value);
  return idx;
}

void
emit(compiler_unit *u, perl_instruction i)
{
  if (u->fill == u->max) {
    u->max *= 2;
    u->code = realloc(u->code, sizeof(perl_instruction)*u->max);
  }
  u->code[u->fill++] = i;
}

void
compile_sassign(perl_compiler *c, compiler_unit *u, perl_node *first, int reg)
{
  switch (first->type) {
    case NODE_SCALARVAR:
      {
        node_variable *left = to_node_variable(first);
          peep(c, u, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0), reg);
          //emit(c->u, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0));
      }
      break;
    default:
      break;
  }
}

int
add_symbol(perl_compiler *c, compiler_unit *u, perl_node *n)
{
  node_sym *k = to_node_sym(n);

  int idx = perl_array_length(c->state, u->syms);
  perl_array_push(c->state, u->syms, k->sym);
  return idx;
}

static
void
compile(perl_compiler *c, compiler_unit *u, perl_node *n)
{
  if (!n) return;

  compiler_unit *prev = u;

  switch (n->type) {
    default:
      break;
    case NODE_PROGRAM:
      {
        node_program *program = to_node_program(n);
        u = compiler_unit_new(c, NULL, program->program);
        compile(c, u, program->program);
        perl_code code = compiler_unit_finish(c, u);
        c->code = code;
      }
      break;
    case NODE_STATEMENTLIST:
      {
        node_statementlist *stmt = to_node_statementlist(n);
        node_statementlist *iter;
        for (iter = stmt; iter; iter = to_node_statementlist(iter->next)) {
          compile(c, u, iter->statement);
        }
      }
      break;
    case NODE_STATEMENT:
      {
        node_statement *stmt = to_node_statement(n);
        compile(c, u, stmt->expr);
      }
      break;
    case NODE_CALL:
      {
        node_call *call = to_node_call(n);
        compile(c, u, call->args);
        compile(c, u, call->name);
        emit(u, CREATE_ABC(OP_ENTERSUB, u->reg, 0, 0));
      }
      break;
    case NODE_PACKAGE:
      {
        node_package *package = to_node_package(n);
        perl_scalar *stash = perl_hash_fetch(c->state, c->defstash, perl_str_cat_cstr(c->state, perl_str_copy(c->state, to_node_sym(package->name)->sym), "::", 2), perl_undef_new(), false);
        if (stash != NULL) {
          c->curstash = *stash;
        }
      }
      break;
    case NODE_SUB:
      {
        node_sub *sub = to_node_sub(n);
        u = compiler_unit_new(c, u, sub->subbody);
        compile(c, u, sub->subbody);
        perl_code code = compiler_unit_finish(c, u);
      }
      break;
    case NODE_SASSIGN:
      {
        node_binop *binop = to_node_binop(n);
        compile(c, u, binop->last);
        u->reg--;
        compile_sassign(c, u, binop->first, u->reg);
      }
      break;
    case NODE_AASSIGN:
      {

      }
      break;
    case NODE_BLOCK:
      {
        compile(c, u, to_node_block(n)->statementlist);
      }
      break;
    case NODE_SCALARVAR:
      {
        node_variable *v = to_node_variable(n);
        emit(u, CREATE_ABC(OP_SASSIGN, u->reg, v->variable->idx, 0));
        u->reg++;
      }
      break;
    case NODE_CONST:
      {
        int idx = add_const(c, u, n);
        emit(u, CREATE_ABC(OP_CONST, u->reg ? u->reg : 0, idx, 0));
        u->reg++;
      }
      break;
    case NODE_METHOD_CALL:
      {
        node_method_call *call = (node_method_call *)n;
        compile(c, u, call->invocant);
        compile(c, u, call->name);
        compile(c, u, call->args);
        emit(u, CREATE_ABC(OP_METHOD, 0, 0, 0));
      }
      break;
    case NODE_SYM:
      {
      	node_sym *sym = (node_sym *)n;

        emit(u, CREATE_ABC(OP_SASSIGN, u->reg, 0, 0));
				u->reg++;
        emit(u, CREATE_ABC(OP_GV, u->reg, 0, 0));
				u->reg++;
      }
      break;
  }
}

perl_code
perl_compile(perl_state *state, perl_node *n)
{
  perl_compiler *c;

  if (n == NULL) return perl_undef_new();

  c = perl_compiler_new(state);
  c->stash_stack = perl_array_new(state);

  compile(c, NULL, n);
  
  if (state->options & PERL_OPTION_OPCODE) {
    perl_code_dump(state, c->code);
  }
  return c->code;
}

perl_compiler *
perl_compiler_new(perl_state *state)
{
  perl_compiler *c;

  c = malloc(sizeof(perl_compiler));
  c->state = state;
  c->defstash = state->defstash;
  c->curstash = c->defstash;

  return c;
}

void
perl_code_dump(perl_state *state, perl_code code)
{
	struct perl_code *c = perl_to_code(code);
  int i, op;

  for (i = 0; i < c->size; i++) {
    perl_instruction inst = c->code[i];
    switch (GET_OPCODE(inst)) {
      case OP_ENTERSUB:
        printf("OP_ENTERSUB\tR%d\tR%d\n", GETARG_A(inst), GETARG_B(inst));
        break;
      case OP_ENTER:
        printf("OP_ENTER\tR%d\tR%d\n", GETARG_A(inst), GETARG_B(inst));
        break;
      case OP_METHOD:
        printf("OP_METHOD\tR%d\tR%d\n", GETARG_A(inst), GETARG_B(inst));
        break;
      case OP_CONST:
        printf("OP_CONST\tR%d\tCONSTS(%d)\n", GETARG_A(inst), GETARG_B(inst));
        break;
      case OP_SASSIGN:
        printf("OP_SASSIGN\tR%d\tR%d\n", GETARG_A(inst), GETARG_B(inst));
        break;
      case OP_GV:
        printf("OP_GV\tR%d\tR%d\n", GETARG_A(inst), GETARG_B(inst));
        break;
      default:
        printf("Compiling UNKNOWN OP = %d\n", GET_OPCODE(inst));
        break;
    }
  }
}

