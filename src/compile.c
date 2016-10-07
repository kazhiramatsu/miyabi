#include "miyabi/compile.h"
#include "miyabi/keywords.h"

struct perl_compiler {
  perl_state *state;
  perl_compiler *prev;
  perl_instruction *inst;
  int fill;
  int max;
  int reg;
  perl_array constants;
  perl_array syms;
  perl_array closures;
  struct perl_code *code;
};

static const keyword builtins[] = {
  { "shift", OP_SHIFT },
  { "print", OP_PRINT },
  { "open",  OP_OPEN },
};

static
int
reserved_reg(perl_variable *n)
{
  int size = 0;
  perl_variable *v = NULL;

  if (!n) return size;

  for (v = n; v; v = v->next) size++;
  return size;
}

perl_compiler *
perl_compiler_new(perl_state *state, perl_compiler *prev, node *n)
{
  perl_compiler *c;

  c = malloc(sizeof(perl_compiler));
  c->prev = prev;
  c->inst = malloc(sizeof(perl_instruction)*256);
  c->fill = 0;
  c->max = 256;
  c->state = state;
  c->constants = perl_array_new(state);
  c->reg = reserved_reg(((node_block *)n)->variable);
  c->code = malloc(sizeof(struct perl_code));
  c->code->tag = PERL_TAG_CODE;
  c->code->size = 0;
  c->code->code = NULL;
  c->code->constants = perl_array_new(state);
  c->code->cfunc = NULL;

  return c;
}

static void
perl_compiler_finish(perl_compiler *c)
{
  c->code->size = c->fill;
  c->code->code = realloc(c->inst, sizeof(perl_instruction) * c->fill);
  c->code->size = c->fill;
}

void
peep(perl_compiler *c, perl_instruction cur, int reg)
{
  int curop = GET_OPCODE(cur);
  perl_instruction prev = c->inst[c->fill-1];
  int prevop = GET_OPCODE(prev);

  switch (curop) {
    case OP_SASSIGN:
      {
        switch (prevop) {
          case OP_CONST:
            {
              if (GETARG_B(cur) == GETARG_A(prev)) {
                c->inst[c->fill-1] = CREATE_ABC(OP_CONST, GETARG_A(cur), GETARG_B(prev), 0);
              }
            }
        }
      }
  }
}

int
add_constant(perl_compiler *c, perl_node *n)
{
  node_const *k = (node_const *)n;

  int idx = perl_array_length(c->state, c->constants);
  perl_array_push(c->state, c->constants, k->value);
  return idx;
}

void
emit(perl_compiler *c, perl_instruction i)
{
  if (c->fill == c->max) {
    c->max *= 2;
    c->code = realloc(c->code, sizeof(perl_instruction)*c->max);
  }
  c->inst[c->fill++] = i;
}

void
compile_sassign(perl_compiler *c, perl_node *first, int reg)
{
  switch (first->type) {
    case NODE_SCALARVAR:
      {
        node_variable *left = (node_variable *)first;
          peep(c, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0), reg);
          //emit(c->c, CREATE_ABC(OP_SASSIGN, left->variable->idx, reg, 0));
      }
      break;
    default:
      break;
  }
}

int
add_symbol(perl_compiler *c, perl_node *n)
{
  node_identifier *k = (node_identifier *)n;

  int idx = perl_array_length(c->state, c->syms);
  perl_array_push(c->state, c->syms, k->ident);
  return idx;
}

void
compile_identifier(perl_compiler *c, node *n)
{
  int i, j;
  node_call *node = (node_call *)n;

  switch (node->op) {
  case OP_PRINT:
    emit(c, CREATE_ABC(OP_GV, c->reg, 0, 0));
    break;
  default:
    break;
  }
}

static
void
compile(perl_compiler *c, perl_node *n)
{
  if (!n) return;

  perl_compiler *prev = c;

  switch (n->type) {
    default:
      break;
    case NODE_COMP_UNIT:
      {
        c = perl_compiler_new(c->state, c, n);
        node_comp_unit *comp_unit = (node_comp_unit *)n;
        compile(c, comp_unit->statementlist);
        perl_compiler_finish(c);
      }
      break;
    case NODE_STATEMENTLIST:
      {
        node_statementlist *stmt = (node_statementlist *)n;
        node_statement *iter;
        for (iter = (node_statement *)stmt->statement; iter; iter = (node_statement *)iter->next) {
          compile(c, (perl_node *)iter);
        }
      }
      break;
    case NODE_STATEMENT:
      {
        node_statement *stmt = (node_statement *)n;
        compile(c, stmt->expr);
      }
      break;
    case NODE_CALL:
      {
        node_call *call = (node_call *)n;
        compile(c, call->args);
        compile(c, call->name);
        emit(c, CREATE_ABC(OP_ENTERSUB, c->reg, 0, 0));
      }
      break;
    case NODE_PACKAGE:
      {
        node_package *package = (node_package *)n;
        perl_scalar *stash = perl_hash_fetch(c->state, c->state->defstash, perl_str_cat_cstr(c->state, perl_str_copy(c->state, ((node_identifier *)(package->name))->ident), "::", 2), perl_undef_new(), false);
        if (stash != NULL) {
          
        }
      }
      break;
    case NODE_SUB:
      {
        node_sub *sub = (node_sub *)n;
        c = perl_compiler_new(c->state, c, sub->subbody);
        compile(c, sub->subbody);
        perl_compiler_finish(c);
      }
      break;
    case NODE_SASSIGN:
      {
        node_binop *binop = (node_binop *)n;
        compile(c, binop->last);
        c->reg--;
        compile_sassign(c, binop->first, c->reg);
      }
      break;
    case NODE_AASSIGN:
      {

      }
      break;
    case NODE_BLOCK:
      {
        compile(c, ((node_block *)(n))->statementlist);
      }
      break;
    case NODE_SCALARVAR:
      {
        node_variable *v = (node_variable *)n;
        emit(c, CREATE_ABC(OP_SASSIGN, c->reg, v->variable->idx, 0));
        c->reg++;
      }
      break;
    case NODE_CONST:
      {
        int idx = add_constant(c, n);
        emit(c, CREATE_ABC(OP_CONST, c->reg ? c->reg : 0, idx, 0));
        c->reg++;
      }
      break;
    case NODE_METHOD_CALL:
      {
        node_method_call *call = (node_method_call *)n;
        compile(c, call->invocant);
        compile(c, call->name);
        compile(c, call->args);
        emit(c, CREATE_ABC(OP_METHOD, 0, 0, 0));
      }
      break;
    case NODE_IDENTIFIER:
      {
      	node_identifier *ident = (node_identifier *)n;
        emit(c, CREATE_ABC(OP_GV, c->reg, 0, 0));
				c->reg++;
      }
      break;
  }
}

struct perl_code *
perl_compile(perl_state *state, perl_node *n)
{
  perl_compiler *c;

  if (n == NULL) return NULL;

  c = perl_compiler_new(state, NULL, n);

  compile(c, n);
  
  if (state->options & PERL_OPTION_OPCODE) {
    perl_code_dump(state, c->code);
  }
  return c->code;
}

void
perl_code_dump(perl_state *state, struct perl_code *c)
{
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

