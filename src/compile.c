#include "miyabi/compile.h"

struct compiler_unit {
  compiler_unit *prev;
  perl_instruction *code;
  int fill;
  int max;
  int reg;
  perl_array consts;
  perl_array syms;
};

static
int
reserve_reg(perl_node *n)
{
  int size = 0;
  perl_variable *v = NULL;

  if (!n) return size;

  for (v = to_block_node(n)->variable; v; v = v->next) size++;
  return size;
}

static void
compiler_unit_start(perl_compiler *c, compiler_unit *prev, perl_node *n)
{
  compiler_unit *u;

  u = malloc(sizeof(compiler_unit));
  u->prev = prev;
  u->code = malloc(sizeof(perl_instruction)*256);
  u->fill = 0;
  u->max = 256;
  u->consts = perl_array_new(c->state);
  u->reg = reserve_reg(n);
  c->u = u;
}

static perl_code
finish_code(perl_compiler *c)
{
  perl_code result;

  compiler_unit *u = c->u;

  result = perl_code_new(c->state);
  perl_to_code(result)->size = u->fill;
  perl_to_code(result)->code = realloc(u->code, sizeof(perl_instruction) * u->fill);
  perl_to_code(result)->size = u->fill;

	return result;
}

static void
compiler_unit_end(perl_compiler *c)
{
  compiler_unit *u = c->u;

  perl_code code = finish_code(c);
  c->u = u->prev;
}

void
peep(perl_compiler *c, perl_instruction cur, int reg)
{
  int curop = GET_OPCODE(cur);
  compiler_unit *u = c->u;
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
add_const(perl_compiler *c, perl_node *n)
{
  perl_const_node *k = to_const_node(n);

  int idx = perl_array_length(c->state, c->u->consts);
  perl_array_push(c->state, c->u->consts, k->value);
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
compile_sassign(perl_compiler *c, perl_node *first, int reg)
{
  switch (first->type) {
    case NODE_SCALARVAR:
      {
        perl_variable_node *left = to_variable_node(first);
          peep(c, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0), reg);
          //emit(c->u, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0));
      }
      break;
    default:
      break;
  }
}

int
add_symbol(perl_compiler *c, perl_node *n)
{
  perl_sym_node *k = to_sym_node(n);

  int idx = perl_array_length(c->state, c->u->syms);
  perl_array_push(c->state, c->u->syms, k->sym);
  return idx;
}

static
void
compile(perl_compiler *c, perl_node *n)
{
  if (!n) return;

  switch (n->type) {
    default:
      break;
    case NODE_PROGRAM:
      {
        perl_program_node *program = to_program_node(n);
        compile(c, program->program);
      }
      break;
    case NODE_STATEMENTLIST:
      {
        perl_statementlist_node *stmt = to_statementlist_node(n);
        perl_statementlist_node *iter;
        for (iter = stmt; iter; iter = to_statementlist_node(iter->next)) {
          compile(c, iter->statement);
        }
      }
      break;
    case NODE_CALL:
      {
        compiler_unit *u = c->u;
        perl_call_node *call = to_call_node(n);
        compile(c, call->args);
        compile(c, call->name);
        emit(u, CREATE_ABC(OP_ENTERSUB, u->reg, 0, 0));
      }
      break;
    case NODE_PACKAGE:
      {
        perl_package_node *package = to_package_node(n);
        perl_scalar *stash = perl_hash_fetch(c->state, c->defstash, perl_str_cat_cstr(c->state, perl_str_copy(c->state, to_sym_node(package->name)->sym), "::", 2), perl_undef_new(), false);
        if (stash != NULL) {
          c->curstash = *stash;
        } 
      }
      break;
    case NODE_SUB:
      {
        perl_code *code;
        perl_sub_node *sub = to_sub_node(n);
        compiler_unit_start(c, c->u, sub->subbody);
        compile(c, sub->subbody);
        compiler_unit_end(c);
      }
      break;
    case NODE_SASSIGN:
      {
        perl_binop_node *binop = to_binop_node(n);
        compile(c, binop->last);
        c->u->reg--;
        compile_sassign(c, binop->first, c->u->reg);
      }
      break;
    case NODE_AASSIGN:
      {

      }
      break;
    case NODE_BLOCK:
      {
        perl_code *code;
        compiler_unit_start(c, c->u, n);
        compile(c, to_block_node(n)->statementlist);
      }
      break;
    case NODE_SCALARVAR:
      {
        compiler_unit *u = c->u;
        perl_variable_node *v = to_variable_node(n);
        emit(u, CREATE_ABC(OP_SASSIGN, u->reg, v->variable->idx, 0));
        u->reg++;
      }
      break;
    case NODE_CONST:
      {
        int idx = add_const(c, n);
        compiler_unit *u = c->u;
        emit(u, CREATE_ABC(OP_CONST, u->reg ? u->reg : 0, idx, 0));
        u->reg++;
      }
      break;
    case NODE_METHOD_CALL:
      {
        perl_method_call_node *call = (perl_method_call_node *)n;
        compile(c, call->invocant);
        compile(c, call->name);
        compile(c, call->args);
        emit(c->u, CREATE_ABC(OP_METHOD, 0, 0, 0));
      }
      break;
    case NODE_SYM:
      {
      	perl_sym_node *sym = (perl_sym_node *)n;

        emit(c->u, CREATE_ABC(OP_SASSIGN, c->u->reg, 0, 0));
				c->u->reg++;
        emit(c->u, CREATE_ABC(OP_GV, c->u->reg, 0, 0));
				c->u->reg++;
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

  compile(c, n);
  
  perl_code code = finish_code(c);

  perl_code_dump(state, code);
  return code;
}

perl_compiler *
perl_compiler_new(perl_state *state)
{
  perl_compiler *c;

  c = malloc(sizeof(perl_compiler));
  c->state = state;
  c->u = NULL;
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
        printf("UNKNOWN OP = %d\n", GET_OPCODE(inst));
        break;
    }
  }
}

