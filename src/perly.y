%{

#include "miyabi/perl.h"
#include "miyabi/value.h"
#include "miyabi/str.h"
#include "miyabi/array.h"
#include "miyabi/code.h"
#include "miyabi/hash.h"
#include "miyabi/opcode.h"
#include "miyabi/parse.h"
#include "miyabi/node.h"
#include "miyabi/keywords.h"
#include "miyabi/node.h"
#include <stdarg.h>
#include <assert.h>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#define YYMALLOC(n) GC_MALLOC((n))
#define YYFREE(o)   GC_FREE((o))
#define YYSTACK_USE_ALLOCA 0
#define YYLEX_PARAM p

#define isALNUM(c)   (isascii(c) && (isalpha(c) || isdigit(c) || c == '_' || c == '\\'))
#define isIDFIRST(c)    (isALPHA(c) || (c) == '_')
#define isALPHA(c)      (isUPPER(c) || isLOWER(c))
#define isSPACE(c) \
        ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) =='\r' || (c) == '\f')
#define isUPPER(c)      ((c) >= 'A' && (c) <= 'Z')
#define isLOWER(c)      ((c) >= 'a' && (c) <= 'z')

#define isIDFIRST_lazy_if(c) ((*(unsigned char*)c) < 0xc0 && isIDFIRST(*(unsigned char *)c))
#define isXDIGIT(c) (isxdigit((int)(unsigned char)(c)))
#define SPACE_OR_TAB(c) ((c) == ' ' || (c) == '\t')

#define REPORT(retval) perl_token_add(p, retval, &yylval),retval
//#   define REPORT(retval) (retval)
#define preblock(retval) (perl_token_add(p, retval, &yylval), p->bufptr = s, p->expect = XBLOCK, retval)
#define term(retval)     (perl_token_add(p, retval, &yylval), p->bufptr = s, p->expect = XTERM, retval)
#define operator(retval) (perl_token_add(p, retval, &yylval), p->bufptr = s, p->expect = XOPERATOR, retval)
#define AOPERATOR(retval) ao(p, (p->expect = XTERM, p->bufptr = s, retval))
#define token(retval)    (perl_token_add(p, retval, &yylval), p->bufptr = s, retval)
#define ident(retval)    (perl_token_add(p, retval, &yylval), reset(p), retval)
#define eop(f) (yylval.ival=f, perl_token_add(p, EQOP, &yylval), p->expect=XTERM, p->bufptr=s, ((int)EQOP))
#define pmop(f)  (yylval.ival=f, REPORT(MATCHOP), p->expect=XTERM, p->bufptr=s, ((int)MATCHOP))
#define BAop(f)  ao(p, (yylval.ival=f, p->expect=XTERM, p->bufptr=s, REPORT(BITANDOP),(int)BITANDOP))
#define PREREF(retval) (p->expect = XREF,p->bufptr = s, REPORT(retval),retval)
#define BOop(f)  ao(p, (yylval.ival=f, p->expect=XTERM, p->bufptr=s, REPORT(BITOROP), (int)BITOROP))

static const char ident_too_long[] = "Identifier too long";

static const keyword keywords[] = {
  { "sub",     KEY_sub,     false },
  { "package", KEY_package, false },
  { "use",     KEY_use,     false },
  { "my",      KEY_my,      false },
  { "shift",   KEY_shift,   false },
  { "print",   KEY_print,   false },
  { "our",     KEY_our,     false },
  { "open",    KEY_open,    false },
  { "if",      KEY_if,      false },
  { "else",    KEY_else,    false },
  { "elsif",   KEY_elsif,   false },
  { "unless",  KEY_unless,  false },
  { "for",     KEY_for,     false },
  { "local",   KEY_local,   false },
  { "qw",      KEY_qw,      false },
  { "eq",      KEY_eq,      false },
  { "ne",      KEY_ne,      false },
};

/* for debug */
static const keyword tokens[] = {
  { "WORD", WORD },
  { "PACKAGE", PACKAGE },
  { "USE", USE },
  { "MY", MY },
  { "SUB", SUB },
  { "FUNC1", FUNC1 },
  { "FUNC", FUNC },
  { "UNIOP", UNIOP },
  { "LSTOP", LSTOP },
  { "ARROW", ARROW },
  { "HASHBRACK", HASHBRACK },
  { "ADDOP", ADDOP },
  { "IF", IF },
  { "ELSE", ELSE },
  { "ELSIF", ELSIF },
  { "$", '$' },
  { "@", '@' },
  { ";", ';' },
  { "%", '%' },
  { "ASSIGNOP", ASSIGNOP },
  { "{", '{' },
  { "}", '}' },
  { "(", '(' },
  { ")", ')' },
  { "[", '[' },
  { "]", ']' },
  { "+", '+' },
  { "*", '*' },
  { "-", '-' },
  { ",", ',' },
  { "\"", '"' },
  { "&", '&' },
  { "THING", THING },
  { "PRIVATEREF", PRIVATEREF },
  { "FOR", FOR },
  { "LOCAL", LOCAL },
  { "NOAMP", NOAMP },
  { "QWLIST", QWLIST },
  { "METHOD", METHOD },
  { "OP_EQ", OP_EQ },
  { "OP_SNE", OP_SNE },
  { "OP_SEQ", OP_SEQ },
};

static const keyword opcode[] = {
  { "entersub", OP_ENTERSUB },
  { "print", OP_PRINT },
  { "open", OP_OPEN },
  { "shift", OP_SHIFT },
  { "bless", OP_BLESS },
};

static const keyword builtins[] = {
  { "shift", OP_SHIFT },
  { "print", OP_PRINT },
  { "open",  OP_OPEN },
};
%}

%pure_parser
%parse-param {perl_parser *p}
%lex-param {perl_parser *p}

%{
int yyparse(perl_parser *p);
int yylex(void *lval, perl_parser *p);
void yyerror(perl_parser *p, const char *s);
void yywarn(perl_parser *p, const char *s);
void yywarning(perl_parser *p, const char *s);
int perl_yylex(perl_parser *p);
%}

%start prog

%union {
  int ival;
  char *pval;
  node *opval;
}

%token <ival> '{'

%token <opval> WORD METHOD FUNCMETH THING PMFUNC PRIVATEREF QWLIST
%token <opval> FUNC0SUB UNIOPSUB LSTOPSUB
%token <pval> LABEL
%token <ival> FORMAT SUB ANONSUB PACKAGE USE
%token <ival> WHILE UNTIL IF UNLESS ELSE ELSIF CONTINUE FOR
%token <ival> LOOPEX DOTDOT
%token <ival> FUNC0 FUNC1 FUNC UNIOP LSTOP
%token <ival> RELOP EQOP MULOP ADDOP
%token <ival> DOLSHARP DO HASHBRACK NOAMP
%token <ival> LOCAL MY MYSUB
%token COLONATTR

// %type <ival> prog decl format startsub startanonsub startformsub
// %type <ival> progstart remember mremember '&'

%type <opval> use package prog decl format startsub startanonsub startformsub
%type <ival>  '&'
%type <opval> progstart block mblock lineseq line loop cond else remember mremember 
%type <opval> expr term subscripted scalar ary hsh arylen star amper sideff
%type <opval> argexpr nexpr texpr iexpr mexpr mnexpr mtexpr miexpr
%type <opval> listexpr listexprcom indirob listop method
%type <opval> formname subname proto subbody cont my_scalar
%type <opval> subattrlist myattrlist subrout mysubrout myattrterm myterm
%type <opval> termbinop termunop anonymous termdo
%type <pval> label

%nonassoc PREC_LOW
%nonassoc LOOPEX

%left <ival> OROP
%left ANDOP
%right NOTOP
%nonassoc LSTOP LSTOPSUB
%left ','
%right <ival> ASSIGNOP
%right '?' ':'
%nonassoc DOTDOT YADAYADA
%left OROR DORDOR
%left ANDAND
%left <ival> BITOROP
%left <ival> BITANDOP
%nonassoc EQOP
%nonassoc RELOP
%nonassoc UNIOP UNIOPSUB
%left <ival> SHIFTOP
%left ADDOP
%left MULOP
%left <ival> MATCHOP
%right '!' '~' UMINUS REFGEN
%right <ival> POWOP
%nonassoc PREINC PREDEC POSTINC POSTDEC
%left ARROW
%nonassoc <ival> ')'
%left '('
%left '[' '{'

%% /* RULES */

/* The whole program */
prog  :   progstart
    /*CONTINUED*/ lineseq
    { node_program_new(p, node_block_end(p, $1, $2)); }
            /* { $$ = $1; newPROG(block_end($1,$2)); } */
    ;

/* An ordinary block */
block :   '{' remember lineseq '}'
    { $$ = node_block_end(p, $2, $3); }
    ;

remember:   /* NULL */    /* start a full lexical scope */
    { $$ = node_block_start(p); }
    ;

progstart:
    {
      p->expect = XSTATE;
      $$ = node_block_start(p);
    }
    ;

mblock  :   '{' mremember lineseq '}'
    { $$ = node_block_end(p, $2, $3); }
    ;

mremember:    /* NULL */    /* start a partial lexical scope */
    { $$ = node_block_start(p); }
    ;

/* A collection of "lines" in the program */
lineseq :   /* NULL */
    { $$ = NULL; }
    |   lineseq decl
    { $$ = node_append_list(p, $1, $2); }
    |   lineseq line
    { $$ = node_append_list(p, $1, $2); }
    ;

/* A "line" in the program */
line  :   label cond
    { $$ = node_statement_new(p, $1, $2); }
    |   loop  /* loops add their own labels */
    { $$ = node_statement_new(p, NULL, $1); p->expect = XSTATE; }
    |   label ';'
    { }
    |   label sideff ';'
    { $$ = node_statement_new(p, $1, $2); p->expect = XSTATE; }
    ;

/* An expression which may have a side-effect */
sideff  :   error
    { $$ = NULL; }
    |   expr
    { $$ = $1; }
    |   expr IF expr
    { $$ = node_logical_new(p, NODE_AND, $3, $1); }
    |   expr UNLESS expr
    { $$ = node_logical_new(p, NODE_OR, $3, $1); }
    |   expr WHILE expr
    { $$ = node_while_new(p, NODE_WHILE, $3, $1); }
            /* { $$ = newLOOPOP(OPf_PARENS, 1, scalar($3), $1); } */
    |   expr UNTIL iexpr
    { $$ = node_until_new(p, NODE_UNTIL, $3, $1); }
            /* { $$ = newLOOPOP(OPf_PARENS, 1, $3, $1);} */
    |   expr FOR expr
    {}
            /* { $$ = newFOROP(0, Nullch, (line_t)$2, */
            /*        Nullop, $3, $1, Nullop); }       */
    ;

/* else and elsif blocks */
else  :   /* NULL */
    { $$ = NULL; }
    |   ELSE mblock
    { $$ = $2; }
            /* { ($2)->op_flags |= OPf_PARENS; $$ = scope($2); } */
    |   ELSIF '(' mexpr ')' mblock else
    { $$ = node_if_new(p, $3, $5, $6); }
    ;

/* Real conditional expressions */
cond  :   IF '(' remember mexpr ')' mblock else
    { $$ = node_block_end(p, $3, node_if_new(p, $4, $6, $7)); }
            /* { PL_copline = (line_t)$1;        */
            /*     $$ = block_end($3,          */
            /*       newCONDOP(0, $4, scope($6), $7)); } */
    |   UNLESS '(' remember miexpr ')' mblock else
    { $$ = node_block_end(p, $3, node_if_new(p, $4, $6, $7)); }
            /* { PL_copline = (line_t)$1;        */
            /*     $$ = block_end($3,          */
            /*       newCONDOP(0, $4, scope($6), $7)); } */
    ;

/* Continue blocks */
cont  :   /* NULL */
    { $$ = NULL; }
            /* { $$ = Nullop; } */
    |   CONTINUE block
    { $$ = $2; }
            /* { $$ = scope($2); } */
    ;

/* Loops: while, until, for, and a bare block */
loop  :   label WHILE '(' remember mtexpr ')' mblock cont
    {}
            /* { PL_copline = (line_t)$2;      */
            /*     $$ = block_end($4,        */
            /*       newSTATEOP(0, $1,         */
            /*       newWHILEOP(0, 1, (LOOP*)Nullop, */
            /*            $2, $5, $7, $8))); }         */
    |   label UNTIL '(' remember miexpr ')' mblock cont
    {}
            /* { PL_copline = (line_t)$2;      */
            /*     $$ = block_end($4,        */
            /*       newSTATEOP(0, $1,         */
            /*       newWHILEOP(0, 1, (LOOP*)Nullop, */
            /*            $2, $5, $7, $8))); }         */
    |   label FOR MY remember my_scalar '(' mexpr ')' mblock cont
    { $$ = node_block_end(p, $4, node_for_new(p, $5, $7, $9, $10)); }
            /* { $$ = block_end($4,                 */
            /*     newFOROP(0, $1, (line_t)$2, $5, $7, $9, $10)); } */
    |   label FOR scalar '(' remember mexpr ')' mblock cont
    { $$ = node_block_end(p, $5, node_for_new(p, $3, $6, $8, $9)); }
            /* { $$ = block_end($5,                 */
            /*     newFOROP(0, $1, (line_t)$2, mod($3, OP_ENTERLOOP), */
            /*          $6, $8, $9)); }                  */
    |   label FOR '(' remember mexpr ')' mblock cont
    { $$ = node_block_end(p, $4, node_for_new(p, NULL, $5, $7, $8)); }
            /* { $$ = block_end($4,                  */
            /*     newFOROP(0, $1, (line_t)$2, Nullop, $5, $7, $8)); } */
    |   label FOR '(' remember mnexpr ';' mtexpr ';' mnexpr ')' mblock
    {}
            /* basically fake up an initialize-while lineseq */
            /* { OP *forop;                                 */
            /*   PL_copline = (line_t)$2;                         */
            /*   forop = newSTATEOP(0, $1,                          */
            /*          newWHILEOP(0, 1, (LOOP*)Nullop,                    */
            /*            $2, scalar($7),                             */
            /*            $11, $9));                                */
            /*   if ($5) {                                  */
            /*    forop = append_elem(OP_LINESEQ,                       */
   /*                    newSTATEOP(0, ($1?savepv($1):Nullch), */
            /*               $5),                                 */
            /*        forop);                                  */
            /*   }                                      */

            /*   $$ = block_end($4, forop); }                       */
    |   label block cont  /* a block is a loop that happens once */
    { $$ = $2;}
            /* { $$ = newSTATEOP(0, $1,      */
            /*     newWHILEOP(0, 1, (LOOP*)Nullop, */
            /*          NOLINE, Nullop, $2, $3)); } */
    ;

/* Normal expression */
nexpr :   /* NULL */
    {}
            /* { $$ = Nullop; } */
    |   sideff
    ;

/* Boolean expression */
texpr :   /* NULL means true */
    {}
            /* { (void)scan_num("1", &yylval); $$ = yylval.opval; } */
    |   expr
    ;

/* Inverted boolean expression */
iexpr :   expr
    {}
            /* { $$ = invert(scalar($1)); } */
    ;

/* Expression with its own lexical scope */
mexpr :   expr
    { $$ = $1;}
            /* { $$ = $1; intro_my(); } */
    ;

mnexpr  :   nexpr
    {}
            /* { $$ = $1; intro_my(); } */
    ;

mtexpr  :   texpr
    {}
            /* { $$ = $1; intro_my(); } */
    ;

miexpr  :   iexpr
    {}
            /* { $$ = $1; intro_my(); } */
    ;

/* Optional "MAIN:"-style loop labels */
label :   /* empty */
    { $$ = NULL; }
            /* { $$ = Nullch; } */
    |   LABEL
    { $$ = NULL; }
    ;

/* Some kind of declaration - does not take part in the parse tree */
decl  :   format
    {  }
            /* { $$ = 0; } */
    |   subrout
    { $$ = $1; }
            /* { $$ = 0; } */
    |   mysubrout
    {  }
            /* { $$ = 0; } */
    |   package
    { $$ = $1; }
            /* { $$ = 0; } */
    |   use
    { $$ = $1; }
            /* { $$ = 0; } */
    ;

format  :   FORMAT startformsub formname block
    {}
            /* { newFORM($2, $3, $4); } */
    ;

formname:   WORD      { /*$$ = $1;*/ }
    {}
    |   /* NULL */    { /*$$ = Nullop;*/ }
    {}
    ;

/* Unimplemented "my sub foo { }" */
mysubrout:    MYSUB startsub subname proto subattrlist subbody
    {}
            /* { newMYSUB($2, $3, $4, $5, $6); } */
    ;

/* Subroutine definition */
subrout :   SUB startsub subname proto subattrlist subbody
    { $$ = node_sub_new(p, $2, $3, $4, $5, $6); }
            /* { newATTRSUB($2, $3, $4, $5, $6); } */
    ;

startsub:   /* NULL */    /* start a regular subroutine scope */
    { $$ = NULL; }
            /* { $$ = start_subparse(FALSE, 0); } */
    ;

startanonsub: /* NULL */    /* start an anonymous subroutine scope */
    {}
            /* { $$ = start_subparse(FALSE, CVf_ANON); } */
    ;

startformsub: /* NULL */    /* start a format subroutine scope */
    {}
            /* { $$ = start_subparse(TRUE, 0); } */
    ;

/* Name of a subroutine - must be a bareword, could be special */
subname :   WORD
    { $$ = $1; }
   /*    {                            */
   /*    STRLEN n_a; char *name = SvPV(((SVOP*)$1)->op_sv,n_a); */
              /* if (strEQ(name, "BEGIN") || strEQ(name, "END")        */
              /*   || strEQ(name, "INIT") || strEQ(name, "CHECK"))     */
              /*   CvSPECIAL_on(PL_compcv);                */
              /* $$ = $1; }                          */
    ;

/* Subroutine prototype */
proto :   /* NULL */
    { $$ = NULL; }
            /* { $$ = Nullop; } */
    |   THING
    ;

/* Optional list of subroutine attributes */
subattrlist:  /* NULL */
    { $$ = NULL; }
            /* { $$ = Nullop; } */
    |   COLONATTR THING
    {}
            /* { $$ = $2; } */
    |   COLONATTR
    {}
            /* { $$ = Nullop; } */
    ;

/* List of attributes for a "my" variable declaration */
myattrlist:   COLONATTR THING
    {}
            /* { $$ = $2; } */
    |   COLONATTR
    {}
            /* { $$ = Nullop; } */
    ;

/* Subroutine body - either null or a block */
subbody :   block { $$ = $1; p->expect = XSTATE; }
    |   ';'   {/* $$ = Nullop; PL_expect = XSTATE;*/ }
    ;

package :   PACKAGE WORD WORD ';'
    { $$ = node_package_new(p, $3); }
    |   PACKAGE ';'
    {}
            /* { package(Nullop); } */
    ;

use   :   USE startsub
    { }
             //{ CvSPECIAL_on(PL_compcv); /* It's a BEGIN {} */ }
        WORD WORD listexpr ';'
    { $$ = node_use_new(p, $1, $2, $4, $5, $6); }
            /* { utilize($1, $2, $4, $5, $6); } */
    ;

/* Ordinary expressions; logical combinations */
expr  :   expr ANDOP expr
    { $$ = node_logical_new(p, NODE_AND, $1, $3); }
            /* { $$ = newLOGOP(OP_AND, 0, $1, $3); } */
    |   expr OROP expr
    { $$ = node_logical_new(p, NODE_OR, $1, $3); }
            /* { $$ = newLOGOP($2, 0, $1, $3); } */
    |   argexpr %prec PREC_LOW
    { $$ = $1; }
    ;

/* Expressions are a list of terms joined by commas */
argexpr :   argexpr ','
    { $$ = $1; }
            /* { $$ = $1; } */
    |   argexpr ',' term
    { $$ = node_append_elem(p, NODE_LIST, $1, $3); }
            /* { $$ = append_elem(OP_LIST, $1, $3); } */
    |   term %prec PREC_LOW
    { $$ = $1;}
    ;

/* List operators */
listop  :   LSTOP indirob argexpr /* map {...} @args or print $fh @args */
    { $$ = node_builtin_call_new(p, NODE_CALL, $1, $2, $3); }
            /* { $$ = convert($1, OPf_STACKED,          */
            /*    prepend_elem(OP_LIST, newGVREF($1,$2), $3) ); } */
    |   FUNC '(' indirob expr ')'    /* print ($fh @args */
    { $$ = node_builtin_call_new(p, NODE_CALL, $1, $3, $4); }
            /* { $$ = convert($1, OPf_STACKED,          */
            /*    prepend_elem(OP_LIST, newGVREF($1,$3), $4) ); } */
    |   term ARROW method '(' listexprcom ')' /* $foo->bar(list) */
    { $$ = node_method_call_new(p, NODE_METHOD_CALL, $1, $3, $5); }
            /* { $$ = convert(OP_ENTERSUB, OPf_STACKED,    */
            /*    append_elem(OP_LIST,             */
            /*      prepend_elem(OP_LIST, scalar($1), $5), */
            /*      newUNOP(OP_METHOD, 0, $3))); }       */
    |   term ARROW method           /* $foo->bar */
    { $$ = node_method_call_new(p, NODE_METHOD_CALL, $1, $3, NULL); }
            /* { $$ = convert(OP_ENTERSUB, OPf_STACKED, */
            /*    append_elem(OP_LIST, scalar($1),    */
            /*      newUNOP(OP_METHOD, 0, $3))); }    */
    |   METHOD indirob listexpr        /* new Class @args */
    { $$ = node_method_call_new(p, NODE_METHOD_CALL, $1, $2, $3);}
            /* { $$ = convert(OP_ENTERSUB, OPf_STACKED, */
            /*    append_elem(OP_LIST,          */
            /*      prepend_elem(OP_LIST, $2, $3),    */
            /*      newUNOP(OP_METHOD, 0, $1))); }    */
    |   FUNCMETH indirob '(' listexprcom ')' /* method $object (@args) */
    { $$ = node_method_call_new(p, NODE_METHOD_CALL, $1, $2, $4); }
            /* { $$ = convert(OP_ENTERSUB, OPf_STACKED, */
            /*    append_elem(OP_LIST,          */
            /*      prepend_elem(OP_LIST, $2, $4),    */
            /*      newUNOP(OP_METHOD, 0, $1))); }    */
    |   LSTOP listexpr             /* print @args */
    { $$ = node_builtin_call_new(p, NODE_CALL, $1, NULL, $2); }
            /* { $$ = convert($1, 0, $2); } */
    |   FUNC '(' listexprcom ')'       /* print (@args) */
    { $$ = node_builtin_call_new(p, NODE_CALL, $1, NULL, $3); }
            /* { $$ = convert($1, 0, $3); } */
    |   LSTOPSUB startanonsub block /* sub f(&@); f { foo } ... */
    {}
            /* { $3 = newANONATTRSUB($2, 0, Nullop, $3); } */
          listexpr      %prec LSTOP  /* ... @bar */
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,   */
            /*     append_elem(OP_LIST,           */
            /*       prepend_elem(OP_LIST, $3, $5), $1)); } */
    ;

/* Names of methods. May use $object->$methodname */
method  :   METHOD
    { $$ = $1; }
    |   scalar
    { $$ = $1;}
    ;

/* Some kind of subscripted expression */
subscripted:  star '{' expr ';' '}'    /* *main::{something} */
            /* In this and all the hash accessors, ';' is
             * provided by the tokeniser */
    { p->expect = XOPERATOR; }
            /* { $$ = newBINOP(OP_GELEM, 0, $1, scalar($3)); */
            /*     PL_expect = XOPERATOR; }          */
    |   scalar '[' expr ']'      /* $array[$element] */
    { $$ = node_aelem_new(p, $1, $3); }
            /* { $$ = newBINOP(OP_AELEM, 0, oopsAV($1), scalar($3)); } */
    |   term ARROW '[' expr ']'    /* somearef->[$element] */
    {}
            /* { $$ = newBINOP(OP_AELEM, 0,  */
            /*        ref(newAVREF($1),OP_RV2AV), */
            /*        scalar($4));}       */
    |   subscripted '[' expr ']'  /* $foo->[$bar]->[$baz] */
    {}
            /* { $$ = newBINOP(OP_AELEM, 0,  */
            /*        ref(newAVREF($1),OP_RV2AV), */
            /*        scalar($3));}       */
    |   scalar '{' expr ';' '}'    /* $foo->{bar();} */
    { p->expect = XOPERATOR; }
            /* { $$ = newBINOP(OP_HELEM, 0, oopsHV($1), jmaybe($3)); */
            /*     PL_expect = XOPERATOR; }              */
    |   scalar '{' expr '}'    /* $foo->{bar();} */
    {}
    |   term ARROW '{' expr ';' '}' /* somehref->{bar();} */
    { p->expect = XOPERATOR; }
            /* { $$ = newBINOP(OP_HELEM, 0,  */
            /*        ref(newHVREF($1),OP_RV2HV), */
            /*        jmaybe($4));        */
            /*     PL_expect = XOPERATOR; }  */
    |   subscripted '{' expr ';' '}' /* $foo->[bar]->{baz;} */
    { p->expect = XOPERATOR; }
            /* { $$ = newBINOP(OP_HELEM, 0,  */
            /*        ref(newHVREF($1),OP_RV2HV), */
            /*        jmaybe($3));        */
            /*     PL_expect = XOPERATOR; }  */
    |   term ARROW '(' ')'      /* $subref->() */
    { }
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, */
            /*       newCVREF(0, scalar($1))); }      */
    |   term ARROW '(' expr ')'   /* $subref->(@args) */
    { }
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, */
            /*       append_elem(OP_LIST, $4,       */
            /*         newCVREF(0, scalar($1)))); }   */

    |   subscripted '(' expr ')'   /* $foo->{bar}->(@args) */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, */
            /*       append_elem(OP_LIST, $3,       */
            /*             newCVREF(0, scalar($1)))); }    */
    |   subscripted '(' ')'      /* $foo->{bar}->() */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, */
            /*       newCVREF(0, scalar($1))); }      */
  ;

/* Binary operators between terms */
termbinop   :   term ASSIGNOP term         /* $x = $y */
    { $$ = node_assign_new(p, $1, $3); }
            /* { $$ = newASSIGNOP(OPf_STACKED, $1, $2, $3); } */
    |   term POWOP term              /* $x ** $y */
    {}
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term MULOP term              /* $x * $y, $x x $y */
    {}
            /* {   if ($2 != OP_REPEAT)            */
            /*    scalar($1);                  */
            /*     $$ = newBINOP($2, 0, $1, scalar($3)); } */
    |   term ADDOP term              /* $x + $y */
    { }
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term SHIFTOP term            /* $x >> $y, $x << $y */
    {}
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term RELOP term              /* $x > $y, etc. */
    {}
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term EQOP term               /* $x == $y, $x eq $y */
    { $$ = node_eq_new(p, $2, $1, $3); }
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term BITANDOP term             /* $x & $y */
    {}
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term BITOROP term            /* $x | $y */
    {}
            /* { $$ = newBINOP($2, 0, scalar($1), scalar($3)); } */
    |   term DOTDOT term             /* $x..$y, $x...$y */
    {}
            /* { $$ = newRANGE($2, scalar($1), scalar($3));} */
    |   term ANDAND term             /* $x && $y */
    { $$ = node_logical_new(p, NODE_AND, $1, $3); }
            /* { $$ = newLOGOP(OP_AND, 0, $1, $3); } */
    |   term OROR term               /* $x || $y */
    { $$ = node_logical_new(p, NODE_OR, $1, $3); }
            /* { $$ = newLOGOP(OP_OR, 0, $1, $3); } */
    |   term MATCHOP term            /* $x =~ /$y/ */
    {}
            /* { $$ = bind_match($2, $1, $3); } */
  ;

/* Unary operators and terms */
termunop : '-' term %prec UMINUS             /* -$x */
    {  $$ = node_unop_new(p, NODE_NEGATE, $2); }
            /* { $$ = newUNOP(OP_NEGATE, 0, scalar($2)); } */
    |   '+' term %prec UMINUS          /* +$x */
    {}
            /* { $$ = $2; } */
    |   '!' term                 /* !$x */
    {}
            /* { $$ = newUNOP(OP_NOT, 0, scalar($2)); } */
    |   '~' term                 /* ~$x */
    {}
            /* { $$ = newUNOP(OP_COMPLEMENT, 0, scalar($2));} */
    |   term POSTINC               /* $x++ */
    {}
            /* { $$ = newUNOP(OP_POSTINC, 0,   */
            /*        mod(scalar($1), OP_POSTINC)); } */
    |   term POSTDEC               /* $x-- */
    {}
            /* { $$ = newUNOP(OP_POSTDEC, 0,   */
            /*        mod(scalar($1), OP_POSTDEC)); } */
    |   PREINC term                /* ++$x */
    {}
            /* { $$ = newUNOP(OP_PREINC, 0,   */
            /*        mod(scalar($2), OP_PREINC)); } */
    |   PREDEC term                /* --$x */
    {}
            /* { $$ = newUNOP(OP_PREDEC, 0,   */
            /*        mod(scalar($2), OP_PREDEC)); } */

  ;

/* Constructors for anonymous data */
anonymous:    '[' expr ']'
    { $$ = node_anonlist_new(p, $2); }
            /* { $$ = newANONLIST($2); } */
    |   '[' ']'
    { $$ = node_anonlist_new(p, NULL); }
            /* { $$ = newANONLIST(Nullop); } */
    |   HASHBRACK expr ';' '}'  %prec '(' /* { foo => "Bar" } */
    { $$ = node_anonhash_new(p, $2); }
            /* { $$ = newANONHASH($2); } */
    |   HASHBRACK expr  '}'  %prec '(' /* { foo => "Bar" } */
    { $$ = node_anonhash_new(p, $2); }
    |   HASHBRACK ';' '}'   %prec '(' /* { } (';' by tokener) */
    { $$ = node_anonhash_new(p, NULL); }
    |   HASHBRACK '}'   %prec '(' /* { } (';' by tokener) */
    { $$ = node_anonhash_new(p, NULL); }
            /* { $$ = newANONHASH(Nullop); } */
    |   ANONSUB startanonsub proto subattrlist block  %prec '('
    {}
            /* { $$ = newANONATTRSUB($2, $3, $4, $5); } */

  ;

/* Things called with "do" */
termdo  :   DO term %prec UNIOP           /* do $filename */
    {}
            /* { $$ = dofile($2); } */
    |   DO block    %prec '('       /* do { code */
    {}
            /* { $$ = newUNOP(OP_NULL, OPf_SPECIAL, scope($2)); } */
    |   DO WORD '(' ')'             /* do somesub() */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB,  */
            /*     OPf_SPECIAL|OPf_STACKED, */
            /*     prepend_elem(OP_LIST,  */
            /*    scalar(newCVREF(      */
            /*      (OPpENTERSUB_AMPER<<8), */
            /*      scalar($2)        */
            /*    )),Nullop)); dep();}    */
    |   DO WORD '(' expr ')'          /* do somesub(@args) */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB,  */
            /*     OPf_SPECIAL|OPf_STACKED, */
            /*     append_elem(OP_LIST,   */
            /*    $4,             */
            /*    scalar(newCVREF(      */
            /*      (OPpENTERSUB_AMPER<<8), */
            /*      scalar($2)        */
            /*    )))); dep();}       */
    |   DO scalar '(' ')'            /* do $subref () */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_SPECIAL|OPf_STACKED, */
            /*     prepend_elem(OP_LIST,              */
            /*    scalar(newCVREF(0,scalar($2))), Nullop)); dep();} */
    |   DO scalar '(' expr ')'           /* do $subref (@args) */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_SPECIAL|OPf_STACKED, */
            /*     prepend_elem(OP_LIST,              */
            /*    $4,                         */
            /*    scalar(newCVREF(0,scalar($2))))); dep();}     */

    ;

term  :   termbinop
    |   termunop
    |   anonymous
    { $$ = $1; }
    |   termdo
    |   term '?' term ':' term
    {}
            /* { $$ = newCONDOP(0, $1, $3, $5); } */
    |   REFGEN term              /* \$x, \@y, \%z */
    {}
            /* { $$ = newUNOP(OP_REFGEN, 0, mod($2,OP_REFGEN)); } */
    |   myattrterm    %prec UNIOP
    { $$ = $1; }
            /* { $$ = $1; } */
    |   LOCAL term    %prec UNIOP
    { $$ = $2; }
            /* { $$ = localize($2,$1); } */
    |   '(' expr ')'
    { $$ = perl_sawparens(p, $2); }
            /* { $$ = sawparens($2); } */
    | QWLIST
    { $$ = $1; }
    |   '(' ')'
    { $$ = perl_sawparens(p, node_nulllist_new(p)); }
            /* { $$ = sawparens(newNULLLIST()); } */
    |   scalar  %prec '('
    { $$ = $1; }
            /* { $$ = $1; } */
    |   star  %prec '('
    { $$ = $1; }
            /* { $$ = $1; } */
    |   hsh   %prec '('
    { $$ = $1; }
            /* { $$ = $1; } */
    |   ary   %prec '('
    { $$ = $1; }
            /* { $$ = $1; } */
    |   arylen  %prec '('          /* $#x, $#{ something } */
    {}
            /* { $$ = newUNOP(OP_AV2ARYLEN, 0, ref($1, OP_AV2ARYLEN));} */
    |   subscripted
    { $$ = $1; }
            /* { $$ = $1; } */
    |   '(' expr ')' '[' expr ']'      /* list slice */
    {}
            /* { $$ = newSLICEOP(0, $5, $2); } */
          | QWLIST '[' expr ']'            /* list literal slice */
    { }
    |   '(' ')' '[' expr ']'         /* empty list slice! */
    {}
            /* { $$ = newSLICEOP(0, $4, Nullop); } */
    |   ary '[' expr ']'           /* array slice */
    {}
            /* { $$ = prepend_elem(OP_ASLICE, */
            /*    newOP(OP_PUSHMARK, 0),      */
            /*      newLISTOP(OP_ASLICE, 0,   */
            /*        list($3),          */
            /*        ref($1, OP_ASLICE))); }    */
    |   ary '{' expr ';' '}'         /* @hash{@keys} */
    { p->expect = XOPERATOR; }
            /* { $$ = prepend_elem(OP_HSLICE,  */
            /*    newOP(OP_PUSHMARK, 0),       */
            /*      newLISTOP(OP_HSLICE, 0,    */
            /*        list($3),           */
            /*        ref(oopsHV($1), OP_HSLICE))); */
            /*     PL_expect = XOPERATOR; }    */
    |   THING %prec '('
            { $$ = $1; }
    |   amper                /* &foo; */
    { $$ = node_call_new(p, NODE_CALL, $1, NULL); }
            /* { $$ = newUNOP(OP_ENTERSUB, 0, scalar($1)); } */
    |   amper '(' ')'            /* &foo() */
    { $$ = node_call_new(p, NODE_CALL, $1, NULL); }
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($1)); } */
    |   amper '(' expr ')'           /* &foo(@args) */
    { $$ = node_call_new(p, NODE_CALL, $1, $3); }
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,   */
            /*     append_elem(OP_LIST, $3, scalar($1))); } */
    |   NOAMP WORD listexpr          /* foo(@args) */
    { $$ = NULL; }
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,   */
            /*     append_elem(OP_LIST, $3, scalar($2))); } */
    |   LOOPEX  /* loop exiting command (goto, last, dump, etc) */
    {}
            /* { $$ = newOP($1, OPf_SPECIAL);    */
            /*     PL_hints |= HINT_BLOCK_SCOPE; } */
    |   LOOPEX term
    {}
            /* { $$ = newLOOPEX($1,$2); } */
    |   NOTOP argexpr            /* not $foo */
    {}
            /* { $$ = newUNOP(OP_NOT, 0, scalar($2)); } */
    |   UNIOP                /* Unary op, $_ implied */
    { $$ = node_unary_call_new(p, NODE_CALL, $1); }
            /* { $$ = newOP($1, 0); } */
    |   UNIOP block              /* eval { foo }, I *think* */
    {}
            /* { $$ = newUNOP($1, 0, $2); } */
    |   UNIOP term               /* Unary op */
    {}
            /* { $$ = newUNOP($1, 0, $2); } */
    |   UNIOPSUB term            /* Sub treated as unop */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,   */
            /*     append_elem(OP_LIST, $2, scalar($1))); } */
    |   FUNC0                /* Nullary operator */
    {}
            /* { $$ = newOP($1, 0); } */
    |   FUNC0 '(' ')'
    {}
            /* { $$ = newOP($1, 0); } */
    |   FUNC0SUB               /* Sub treated as nullop */
    {}
            /* { $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, */
            /*    scalar($1)); }              */
    |   FUNC1 '(' ')'            /* not () */
    {}
            /* { $$ = $1 == OP_NOT ? newUNOP($1, 0, newSVOP(OP_CONST, 0, newSViv(0))) */
            /*          : newOP($1, OPf_SPECIAL); }                    */
    |   FUNC1 '(' expr ')'           /* not($foo) */
    {}
            /* { $$ = newUNOP($1, 0, $3); } */
    |   PMFUNC '(' term ')'          /* split (/foo/) */
    {}
            /* { $$ = pmruntime($1, $3, Nullop); } */
    |   PMFUNC '(' term ',' term ')'     /* split (/foo/,$bar) */
    {}
            /* { $$ = pmruntime($1, $3, $5); } */
    |   WORD
    { $$ = $1; }
    |   listop
    { $$ = $1; }
    ;

/* "my" declarations, with optional attributes */
myattrterm:   MY myterm myattrlist
    {}
            /* { $$ = my_attrs($2,$3); } */
    |   MY myterm
    { $$ = perl_localize(p, $2, $1); }
            /* { $$ = localize($2,$1); } */
    ;

/* Things that can be "my"'d */
myterm  :   '(' expr ')'
    { $$ = perl_sawparens(p, $2); }
            /* { $$ = sawparens($2); } */
    |   '(' ')'
    { $$ = perl_sawparens(p, node_nulllist_new(p)); }
            /* { $$ = sawparens(newNULLLIST()); } */
    |   scalar  %prec '('
    { $$ = $1;}
            /* { $$ = $1; } */
    |   hsh   %prec '('
    {}
            /* { $$ = $1; } */
    |   ary   %prec '('
    {}
            /* { $$ = $1; } */
    ;

/* Basic list expressions */
listexpr:   /* NULL */ %prec PREC_LOW
    { $$ = NULL; }
            /* { $$ = Nullop; } */
    |   argexpr    %prec PREC_LOW
    { $$ = $1; }
            /* { $$ = $1; } */
    ;

listexprcom:  /* NULL */
    { $$ = NULL; }
            /* { $$ = Nullop; } */
    |   expr
    { $$ = $1; }
            /* { $$ = $1; } */
    |   expr ','
    { $$ = $1; }
            /* { $$ = $1; } */
    ;

/* A little bit of trickery to make "for my $foo (@bar)" actually be
   lexical */
my_scalar:    scalar
    { perl_in_my(p, 0); $$ = $1; }
            /* { PL_in_my = 0; $$ = my($1); } */
    ;

amper :   '&' indirob
    { $$ = $2; }
            /* { $$ = newCVREF($1,$2); } */
    ;

scalar  :   '$' indirob
    {  $$ = node_scalar_ref_new(p, $2); }
            /* { $$ = newSVREF($2); } */

    ;

ary   :   '@' indirob
    {  $$ = node_array_ref_new(p, $2); }
            /* { $$ = newAVREF($2); } */
    ;

hsh   :   '%' indirob
    {  $$ = node_hash_ref_new(p, $2); }
            /* { $$ = newHVREF($2); } */
    ;

arylen  :   DOLSHARP indirob
    {}
            /* { $$ = newAVREF($2); } */
    ;

star  :   '*' indirob
    {}
            /* { $$ = newGVREF(0,$2); } */
    ;

/* Indirect objects */
indirob :   WORD
    { $$ = $1; }
            /* { $$ = scalar($1); } */
    |   scalar %prec PREC_LOW
    {}
            /* { $$ = scalar($1);  } */
    |   block
    {}
            /* { $$ = scope($1); } */

    |   PRIVATEREF
    { $$ = $1; }
            /* { $$ = $1; } */
    ;

%% /* PROGRAM */

#define yylval  (*((YYSTYPE*)(p->ylval)))
#define NEXTVAL_NEXTTOKE (p->nexttoke->nexttokval[p->nexttoke->nexttoken])

typedef struct token_s {
  int type;
  YYSTYPE *lval;
  struct token_s *prev;
  struct token_s *next;
  struct token_s *last;
} token_t;

typedef struct next_token {
  int nexttoken;
  YYSTYPE nexttokval[10];
  int nexttype[10];
} next_token;

token_t *perl_token_new(perl_parser *p, int type, YYSTYPE *lval);
token_t *perl_token_add(perl_parser *p, int type, YYSTYPE *lval);

node *
node_new(perl_parser *p, enum perl_node_type type, uint32_t flags)
{
  node *n;
  
  n = malloc(sizeof(node));
  n->type = type;
  n->flags = flags;
  return n;
}

void
perl_init_node(perl_parser *p, node *base, enum perl_node_type type, int flags)
{
  base->type = type;
  base->flags = flags;
}

node *
node_statementlist_new(perl_parser *p, node *statement)
{
  node_statementlist *n;

  n = malloc(sizeof(node_statementlist));
  perl_init_node(p, &n->base, NODE_STATEMENTLIST, 0);
  n->statement = statement;
  n->next = NULL;
  return (node *)n;
}

node *
node_statement_new(perl_parser *p, char *label, node *expr)
{
  node_statement *n;

  n = malloc(sizeof(node_statement));
  perl_init_node(p, &n->base, NODE_STATEMENT, 0);
  n->expr = expr;
  return (node *)n;
}

node *
node_append_list(perl_parser *p, node *first, node *last)
{
  if (!first) {
    node *new = node_statementlist_new(p, last);
    to_node_statementlist(new)->last = new;
    return new;
  }
  if (!last) {
    return first;
  }
  node_statementlist *n = to_node_statementlist(first);
  node *new = node_statementlist_new(p, last);
  to_node_statementlist(n->last)->next = new;
  n->last = new;
  return first;
}

node *
node_method_call_new(perl_parser *p, enum perl_node_type type, node *invocant, node *name, node *args)
{
  node_method_call *n;

  n = malloc(sizeof(node_method_call));
  perl_init_node(p, &n->base, type, 0);
  n->invocant = invocant;
  n->name = name;
  n->args = args;
  return (node *)n;
}

node *
node_list_new(perl_parser *p, enum perl_node_type type, node *elem)
{
  node_list *n;

  n = malloc(sizeof(node_list));
  perl_init_node(p, &n->base, type, 0);
  n->elem = elem;
  n->next = NULL;
  n->last = NULL;
  return (node *)n;
}

node *
perl_convert(perl_parser *p, enum perl_node_type type, node *list)
{
  if (!list || list->type != NODE_LIST) {
    list = node_list_new(p, type, list);
  }
  list->type = type;
  return list;
}

node *
node_prepend_elem(perl_parser *p, enum perl_node_type type, node *first, node *last)
{
  if (!last) {
    return first;
  }
  if (!first) {
    return node_list_new(p, type, last);
  }
  if (first->type != NODE_LIST) {
    first = node_list_new(p, type, first);
  }
  last = node_list_new(p, type, last);
  to_node_statementlist(last)->next = first;
  return last;
}

node *
node_append_elem(perl_parser *p, enum perl_node_type type, node *first, node *last)
{
  if (!first) {
    node *new = node_list_new(p, type, last);
    to_node_list(new)->last = new;
    return new;
  }
  if (!last) {
    return first;
  }
  if (first->type != NODE_LIST) {
    first = node_list_new(p, type, first);
    node_list *n = to_node_list(first);
    n->last = first;
  }
  node_list *n = to_node_list(first);
  node *new = node_list_new(p, type, last);
  if (!n->last) {
    n->last = new; 
  } else {
    to_node_list(n->last)->next = new;
    n->last = new;
  }
  return first;
}

node *
node_unop_new(perl_parser *p, enum perl_node_type type, node *first)
{
  node_unop *n;

  n = malloc(sizeof(node_unop));
  perl_init_node(p, &n->base, type, 0); 
  n->first = first;
  return (node *)n;
}

static
enum perl_node_type
assign_type(node *first, node *last)
{
  enum perl_node_type t;

  switch (first->type) {
    case NODE_SCALARVAR:
      t = NODE_SASSIGN;
      break;
    case NODE_HASHVAR:
    case NODE_ARRAYVAR:
      t = NODE_AASSIGN;   
      break;
    default:
      t = NODE_SASSIGN;   
      break; 
  }
  return t;
}

node *
node_assign_new(perl_parser *p, node *first, node *last)
{
  node_binop *n;

  n = malloc(sizeof(node_binop));
  perl_init_node(p, &n->base, assign_type(first, last), 0);
  n->first = first;
  n->last = last;
  return (node *)n;
}

static
enum perl_node_type
eqop_type(int eqop)
{
  enum perl_node_type t;

  switch (eqop) {
    case OP_EQ:
      t = NODE_EQ;
      break;
    case OP_SEQ:
      t = NODE_SEQ;   
      break;
    case OP_SNE:
      t = NODE_SNE;   
      break;
    case OP_NE:
      t = NODE_NE;   
      break;
    default:
      break; 
  }
  return t;
}

node *
node_eq_new(perl_parser *p, int eqop, node *first, node *last)
{
  node_binop *n;

  n = malloc(sizeof(node_binop));
  perl_init_node(p, &n->base, eqop_type(eqop), 0);
  n->first = first;
  n->last = last;
  return (node *)n;
}

node *
node_aelem_new(perl_parser *p, node *first, node *last)
{
  node_binop *n;

  n = malloc(sizeof(node_binop));
  perl_init_node(p, &n->base, NODE_AELEM, 0);
  n->first = first;
  n->last = last;
  return (node *)n;
}

node *
node_call_new(perl_parser *p, enum perl_node_type type, node *name, node *args)
{
  node_call *n;

  n = malloc(sizeof(node_call));
  perl_init_node(p, &n->base, NODE_CALL, 0);
  n->name = name;
  n->indirob = NULL;
  n->args = args;
  return (node *)n;
}

node *
node_builtin_call_new(perl_parser *p, enum perl_node_type type, int op, node *indirob, node *args)
{
  node_call *n;

  n = malloc(sizeof(node_call));
  perl_init_node(p, &n->base, NODE_CALL, 0);
  n->op = op;
  n->indirob = indirob;
  n->args = args;
  return (node *)n;
}

node *
node_unary_call_new(perl_parser *p, enum perl_node_type type, int op)
{
  node_call *n;

  n = malloc(sizeof(node_call));
  perl_init_node(p, &n->base, NODE_CALL, 0);
  n->op = op;
  n->name = NULL;
  n->indirob = NULL;
  n->args = NULL;
  return (node *)n;
}

node *
node_sub_new(perl_parser *p, node *startsub, node *subname,
             node *proto, node *subattrlist, node *subbody)
{
  node_sub *n;

  n = malloc(sizeof(node_sub));
  perl_init_node(p, &n->base, NODE_SUB, 0);
  n->startsub = startsub;
  n->subname = subname;
  n->proto = proto;
  n->subattrlist = subattrlist;
  n->subbody = subbody;
  perl_add_our_name(p, p->curstash, to_node_sym(subname)->sym);
  n->stash = p->curstash;
  n->stashname = p->curstashname;

  return (node *)n;
}

node *
node_use_new(perl_parser *p, int aver, node *floor,
             node *version, node *id, node *arg)
{
  node_use *n;

  n = malloc(sizeof(node_use));
  perl_init_node(p, &n->base, NODE_USE, 0);
  n->id = id;
  n->arg = arg;
  return (node *)n;
}

node *
node_package_new(perl_parser *p, node *name)
{
  node_package *n;
  node_sym *v = to_node_sym(name);
  perl_scalar *stash;

  n = malloc(sizeof(node_package));
  perl_init_node(p, &n->base, NODE_PACKAGE, 0);
  n->name = name;
  stash = perl_hash_fetch(p->state, p->defstash,
                          perl_str_new(p->state, p->tokenbuf, p->tokenlen),
                          perl_hash_new(p->state),
                          true);
  if (stash == NULL) {
    perl_add_our_name(p, p->defstash, v->sym);
  }
  p->curstash = *stash;
  p->curstashname = perl_str_cat(p->state, v->sym, perl_str_new(p->state, "::", 2)); 
  return (node *)n;
}

void
yywarn(perl_parser *p, const char *s)
{
  fprintf(stderr, "%s\n", s);
}

void
yyerror(perl_parser *p, const char *s)
{
  //fprintf(stderr, "%s\n", s);
  perl_warner(p->state, "%s line %d\n", s, p->line);
}

node *
node_block_new(perl_parser *p, node *block)
{
  node_block *n;

  n = malloc(sizeof(node_block));
  perl_init_node(p, &n->base, NODE_BLOCK, 0);
  n->outer = block;
  n->variable = NULL;
  n->statementlist = NULL;
  n->k = NULL;
  return (node *)n;
}

node *
node_block_start(perl_parser *p)
{
  node *block;

  block = node_block_new(p, p->curblock);
  p->curblock = block;
  return block;
}

node *
node_block_end(perl_parser *p, node *block, node *statementlist)
{
  if (!block) return NULL;

  node_block *b = (node_block *)block;

  if (b->outer) {
    p->curblock = b->outer;
  }
  b->statementlist = statementlist;
  return (node *)b; 
}

node *
node_for_new(perl_parser *p, node *sv, node *expr, node *block, node *cont)
{
  node_for *n;

  n = malloc(sizeof(node_for));
  perl_init_node(p, &n->base, NODE_FOR, 0);
  n->sv = sv;
  n->expr = expr;
  n->block = block;
  n->cont = cont;
  return (node *)n;
}

node *
node_if_new(perl_parser *p, node *first, node *true_node, node *false_node)
{
  node_logical *logical;
  perl_node *n;

  if (!true_node) {
    return node_logical_new(p, NODE_IF, first, false_node);
  }
  if (!false_node) {
    return node_logical_new(p, NODE_IF, first, true_node);
  }
  n = node_logical_new(p, NODE_IF, first, true_node);
  logical = to_node_logical(n);
  logical->next = false_node;
  return n;
}

node *
node_logical_new(perl_parser *p, enum perl_node_type type, node *first, node *other)
{
  node_logical *n;

  n = malloc(sizeof(node_logical));
  perl_init_node(p, &n->base, type, 0);
  n->first = first;
  n->other = other;
  n->next = NULL;
  return (node *)n;
}

node *
node_while_new(perl_parser *p, enum perl_node_type type, node *cond, node *block)
{
  node_loop *n;

  n = malloc(sizeof(node_loop));
  perl_init_node(p, &n->base, type, 0);
  n->cond = cond;
  n->block = block;
  return (node *)n;
}

node *
node_until_new(perl_parser *p, enum perl_node_type type, node *cond, node *block)
{
  node_loop *n;

  n = malloc(sizeof(node_loop));
  perl_init_node(p, &n->base, type, 0);
  n->cond = cond;
  n->block = block;
  return (node *)n;
}

void
perl_in_my(perl_parser *p, int status)
{
  p->in_my = status;
}

void
perl_lex_state(perl_parser *p, enum lex_state lex_state)
{
  p->lex_state = lex_state;
}

node *
perl_localize(perl_parser *p, node *n, int lex)
{
  p->in_my = false;
  return n;
}

node *
perl_sawparens(perl_parser *p, node *n)
{
  n->flags |= NODE_FLAG_PARENS;
  return n;
}

node *
node_anonlist_new(perl_parser *p, node *first)
{
  node_unop *n;

  n = malloc(sizeof(node_unop));
  perl_init_node(p, &n->base, NODE_ANONLIST, 0);
  n->first = first;
  return (node *)n;
}

node *
node_anonhash_new(perl_parser *p, node *first)
{
  node_unop *n;

  n = malloc(sizeof(node_unop));
  perl_init_node(p, &n->base, NODE_ANONHASH, 0);
  n->first = first;
  return (node *)n;
}

node *
node_nulllist_new(perl_parser *p)
{
  node_list *n;

  n = malloc(sizeof(node_list));
  perl_init_node(p, &n->base, NODE_LIST, 0);
  n->op = 0;
  n->elem = NULL;
  n->next = NULL;
  return (node *)n;
}

node *
node_glob_ref_new(perl_parser *p, node *n)
{
  n->type = NODE_GV;
  return (node *)n;
}

node *
node_scalar_ref_new(perl_parser *p, node *n)
{
  if (n->type == NODE_ANYVAR) {
    n->type = NODE_SCALARVAR;
    return n;
  }
  return n;
}

node *
node_array_ref_new(perl_parser *p, node *name)
{
  node_variable *n;

  n = to_node_variable(name);
  if (n->base.type == NODE_ANYVAR) {
    n->base.type = NODE_ARRAYVAR;
  }
  return (node *)n;
}

node *
node_hash_ref_new(perl_parser *p, node *name)
{
  node_variable *n;

  n = to_node_variable(name);
  if (n->base.type == NODE_ANYVAR) {
    n->base.type = NODE_HASHVAR;
  }
  return (node *)n;
}

perl_variable *
variable_new(perl_scalar name, int idx, enum perl_scope scope)
{
  perl_variable *variable;

  variable = malloc(sizeof(perl_variable));
  variable->name = name;
  variable->idx = idx;
  variable->next = NULL;
  variable->scope = scope; 
  return variable;
}

perl_variable *
perl_add_my_name(perl_parser *p, perl_scalar name)
{
  int idx = 0;
  perl_variable *v, *prev;
  node_block *b = (node_block *)(p->curblock);
 
  if (b->variable == NULL) {
    v = variable_new(name, idx, PERL_SCOPE_MY);
    b->variable = v;
    return v;
  }
  for (v = b->variable; v; prev = v, v = v->next) {
    if (perl_str_eq(NULL, name, v->name)) {
      perl_warner(p->state, "\"%s\" variable %s masks earlier declaration in same scope\n",
                  "my", perl_to_str(v->name)->str);
      return v;
    }
    idx++;
  }
  perl_variable *var = variable_new(name, idx, PERL_SCOPE_MY);
  prev->next = var;
  return var;
}

void
node_program_new(perl_parser *p, node *n)
{
  node_program *prog;

  prog = malloc(sizeof(node_program)); 
  prog->base.type = NODE_PROGRAM;
  prog->program = n;

  p->program = (node *)prog;
}

token_t *
perl_token_new(perl_parser *p, int type, YYSTYPE *lval)
{
  token_t *new;

  new = malloc(sizeof(token_t));
  new->type = type;
  new->lval = lval;
  new->next = NULL;
  new->prev = NULL;
  new->last = NULL;
  return new;
}

token_t *
perl_token_add(perl_parser *p, int type, YYSTYPE *lval)
{
  token_t *new, *iter;

  if (p->tokens == NULL) {
    p->tokens = perl_token_new(p, type, lval);
    p->tokens->last = p->tokens;
    return p->tokens->last;
  }

  new = perl_token_new(p, type, lval);
  p->tokens->last->next = new;
  p->tokens->last = new;
  return new;
}

void
perl_token_dump(perl_parser *p)
{
  int i;
  token_t *iter;

  if (p->tokens == NULL) {
    return;
  }
  for (iter = p->tokens; iter; iter = iter->next) {
    for (i = 0; i < sizeof(tokens)/sizeof(keyword); i++) {
      if (tokens[i].key_id == iter->type) {
        printf("<Token: %s>\n", tokens[i].key_name);
      }
    }
  }
}

void
perl_lex_dump(perl_variable *variable, int indent)
{
  perl_variable *iter;
  int i, j;

  if (!variable) return;
  for (j=0; j<indent; j++) {
      printf("  ");
  }
  printf("lexical variables: ");
  for (iter = variable; iter; iter = iter->next) {
    perl_scalar_dump(iter->name);
    printf(" ");
  }
  printf("\n");
}

int
perl_call_op_dump(node *n, int indent)
{
  int i, j;
  node_call *node = (node_call *)n;

  for (i = 0; i < sizeof(builtins)/sizeof(keyword); i++) {
    if (builtins[i].key_id == node->op) {
      for (j=0; j<indent; j++) {
        printf("  ");
      }
      printf("%s\n", builtins[i].key_name);
    }
  }
  return 1;
}

void
perl_node_dump(node *n, int indent)
{
  int i;

  if (!n) return;
  for (i=0; i<indent; i++) {
    printf("  ");
  }

  switch (n->type) {
    default:
      printf("other node type = %d\n", n->type);
      break;
    case NODE_PROGRAM:
      printf("program:\n");
      {
        node_program *prog = to_node_program(n);
        perl_node_dump(prog->program, indent+1);
      }
      break;
    case NODE_STATEMENTLIST:
      {
        printf("statementlist:\n");
        node_statementlist *stmt = to_node_statementlist(n);
        node_statementlist *iter;
        for (iter = stmt; iter; iter = to_node_statementlist(iter->next)) {
          perl_node_dump(iter->statement, indent+1);
        }
      }
      break;
    case NODE_STATEMENT:
      {
        printf("statement:\n");
        node_statement *stmt = to_node_statement(n);
        perl_node_dump(stmt->expr, indent+1);
      }
      break;
    case NODE_PACKAGE:
      {
        printf("package:\n");
        node_package *package = to_node_package(n);
        perl_node_dump(package->name, indent+1);
      }
      break;
    case NODE_SUB:
      {
        printf("sub:\n");
        node_sub *sub = to_node_sub(n);
        perl_node_dump(sub->subname, indent+1);
        for (int j=0; j<indent+1; j++) {
          printf("  ");
        }
        printf("stash ");
        perl_scalar_dump(sub->stashname);
        printf("\n");
        perl_node_dump(sub->subbody, indent+1);
      }
      break;
    case NODE_SASSIGN:
      printf("sassign:\n");
      {
        node_binop *binop = to_node_binop(n);
        perl_node_dump(binop->first, indent+1);
        perl_node_dump(binop->last, indent+1);
      }
      break;
    case NODE_AASSIGN:
      printf("aassign:\n");
      {
        node_binop *binop = to_node_binop(n);
        perl_node_dump(binop->first, indent+1);
        perl_node_dump(binop->last, indent+1);
      }
      break;
    case NODE_BLOCK:
      printf("block:\n");
      {
        node_block *block = to_node_block(n);
        perl_lex_dump(block->variable, indent+1);
        perl_node_dump(block->statementlist, indent+1);
      }
      break;
    case NODE_SCALARVAR:
      printf("scalar variable ");
      {
        node_variable *v = to_node_variable(n);
        perl_scalar_dump(v->variable->name); 
        printf("\n");
      }
      break;
    case NODE_ARRAYVAR:
      printf("array variable ");
      {
        node_variable *v = to_node_variable(n);
        perl_scalar_dump(v->variable->name); 
        printf("\n");
      }
      break;
    case NODE_HASHVAR:
      printf("hash varriable ");
      {
        node_variable *v = to_node_variable(n);
        perl_scalar_dump(v->variable->name);
        printf("\n");
      }
      break;
    case NODE_CONST:
      printf("const ");
      {
        node_const *v = to_node_const(n);
        perl_scalar_dump(v->value);
        printf("\n");
      }
      break;
    case NODE_NEGATE:
      printf("negate:\n");
      {
        node_unop *v = to_node_unop(n);
        perl_node_dump(v->first, indent+1);
      }
      break;
    case NODE_LIST:
      printf("list\n");
      {
        node_list *l = to_node_list(n);
        node_list *iter;
        for (iter = l; iter; iter = to_node_list(iter->next)) {
          perl_node_dump(iter->elem, indent+1);
        }
      }
      break;
    case NODE_ANONLIST:
      printf("anonlist\n");
      {
        node_unop *l = to_node_unop(n);
        perl_node_dump(l->first, indent+1);
      }
      break;
    case NODE_ANONHASH:
      printf("anonhash\n");
      {
        node_unop *l = to_node_unop(n);
        perl_node_dump(l->first, indent+1);
      }
      break;
    case NODE_USE:
      printf("use\n");
      {
        node_use *p = to_node_use(n);
        perl_node_dump(p->id, indent+1);
        perl_node_dump(p->arg, indent+1);
      }
      break;
    case NODE_AND:
      {
        printf("and\n");
        node_logical *m = to_node_logical(n);
        perl_node_dump(m->first, indent+1);
        perl_node_dump(m->other, indent+1);
        perl_node_dump(m->next, indent+1);
      }
      break;
    case NODE_OR:
      {
        printf("or\n");
        node_logical *m = to_node_logical(n);
        perl_node_dump(m->first, indent+1);
        perl_node_dump(m->other, indent+1);
        perl_node_dump(m->next, indent+1);
      }
      break;
    case NODE_FOR:
      {
        printf("for\n");
        node_for *m = to_node_for(n);
        perl_node_dump(m->sv, indent+1);
        perl_node_dump(m->expr, indent+1);
        perl_node_dump(m->block, indent+1);
        perl_node_dump(m->cont, indent+1);
      }
      break;
    case NODE_AELEM:
      {
        printf("aelem\n");
        node_binop *m = to_node_binop(n);
        perl_node_dump(m->first, indent+1);
        perl_node_dump(m->last, indent+1);
      }
      break;
    case NODE_QWLIST:
      {
        printf("qwlist ");
        node_value *m = to_node_value(n);
        perl_scalar_dump(m->value); 
        printf("\n");
      }
      break;
    case NODE_METHOD_CALL:
      {
        printf("method_call\n");
        node_method_call *m = (node_method_call *)n;
        perl_node_dump(m->invocant, indent+1);
        perl_node_dump(m->name, indent+1);
        perl_node_dump(m->args, indent+1);
      }
      break;
    case NODE_CALL:
      {
        printf("call\n");
        node_call *m = (node_call *)n;
        perl_call_op_dump(n, indent+1);
        perl_node_dump(m->indirob, indent+1);
        perl_node_dump(m->name, indent+1);
        perl_node_dump(m->args, indent+1);
      }
      break;
    case NODE_IF:
      {
        printf("if\n");
        node_logical *m = (node_logical *)n;
        perl_node_dump(m->first, indent+1);
        perl_node_dump(m->other, indent+1);
        perl_node_dump(m->next, indent+1);
      }
      break;
    case NODE_WHILE:
      {
        printf("while\n");
        node_loop *m = (node_loop *)n;
        perl_node_dump(m->cond, indent+1);
        perl_node_dump(m->block, indent+1);
      }
      break;
    case NODE_STR:
      printf("str ");
      {
        node_value *v = to_node_value(n);
        perl_scalar_dump(v->value);
        printf("\n");
      }
      break;
    case NODE_NUMBER:
      printf("number ");
      {
        node_const *v = to_node_const(n);
        perl_scalar_dump(v->value);
        printf("\n");
      }
      break;
    case NODE_SYM:
      printf("sym ");
      {
        node_sym *v = to_node_sym(n);
        perl_scalar_dump(v->sym);
        printf("\n");
      }
      break;
    case NODE_EQ:
      printf("op_eq\n");
      {
        node_binop *v = to_node_binop(n);
        perl_node_dump(v->first, indent+1);
        perl_node_dump(v->last, indent+1);
      }
      break;
    case NODE_SEQ:
      printf("op_seq\n");
      {
        node_binop *v = to_node_binop(n);
        perl_node_dump(v->first, indent+1);
        perl_node_dump(v->last, indent+1);
      }
      break;
    case NODE_NE:
      printf("op_ne\n");
      {
        node_binop *v = to_node_binop(n);
        perl_node_dump(v->first, indent+1);
        perl_node_dump(v->last, indent+1);
      }
      break;
    case NODE_SNE:
      printf("op_sne\n");
      {
        node_binop *v = to_node_binop(n);
        perl_node_dump(v->first, indent+1);
        perl_node_dump(v->last, indent+1);
      }
      break;
  }
}

inline
void
perl_set_lex_state(perl_parser *p, enum lex_state lex_state)
{
  p->lex_state = lex_state;
}

perl_parser *
perl_parse_file(perl_state *state, char *file)
{
  FILE *f;
  perl_parser *p;
  struct stat stat_buf;

  f = fopen(file, "r");
  if (f == NULL) {
    fprintf(stderr, "Cannot open file %s\n", file);
    exit(EXIT_FAILURE);
  }

  p = perl_parser_new(state);
  p->state = state;
  stat(file, &stat_buf);
  p->bufsize = stat_buf.st_size;
  p->linestr = malloc(p->bufsize);
  memset(p->linestr, 0, p->bufsize);
  p->f = f;
  fread(p->linestr, sizeof(char), p->bufsize+1, p->f);
  p->linestr[p->bufsize] = EOF;
  p->nexttoke->nexttoken = 0;

  p->bufptr = p->oldbufptr = p->oldoldbufptr = p->linestart = p->linestr; 
  p->bufend = p->bufptr + p->bufsize;

  yyparse(p);

  if (p->state->options & PERL_OPTION_VERBOSE) {
    perl_token_dump(p);
    perl_node_dump(p->program, 1);
  } else if (p->state->options & PERL_OPTION_AST) {
    perl_node_dump(p->program, 1);
  }
  state->defstash = p->defstash;
  return p;
}

perl_parser *
perl_parser_new(perl_state *state)
{
  perl_parser *p;

  p = malloc(sizeof(*p));
  p->saw_arrow = false;
  p->line = 1;
  p->column = 0;
  p->file = NULL;
  p->state = state;
  p->current = -1;
  p->pending_ident = 0;
  p->in_my = 0;
  p->lex_brackets = 0;
  p->lex_state = LEX_NORMAL;
  p->lex_starts = 0;
  p->expect = XSTATE;
  memset(p->tokenbuf , 0, 1024);
  p->tokenlen = 0;
  p->defstash = perl_hash_new(p->state);
  p->curstash = p->defstash;
  p->curstashname = perl_str_new(p->state, "main::", 6); 
  p->tokens = NULL;
  p->nexttoke = malloc(sizeof(next_token));
  p->nexttoke->nexttoken = 0;
  memset(p->nexttoke->nexttype, 0, 10);
  memset(p->nexttoke->nexttokval, 0, 10);
  p->line_idx = -1;
  p->curblock = NULL;
  p->subname = perl_str_new(p->state, "" ,0);
  return p;
}

int
yylex(void *lval, perl_parser *p)
{
  int t;

  p->ylval = lval;
  t = perl_yylex(p);

  return t;
}

static inline char *
skipspace(perl_parser *p, char *s)
{
  while (1) {
    if (*s != '\n' &&  *s != ' ' && *s != '\t') {
      break;
    }
    s++;
  }
  return s;
}

static inline char * 
skip(perl_parser *p, char *s, char term)
{
  for (;;) {
    if (*s < 0) break;
    if (*s == term) break;
    s++;
  }
  return s;
}

inline static void
save(perl_parser *p, char c)
{
  p->tokenbuf[p->tokenlen++] = c;
}

inline static char *
save_and_next(perl_parser *p, char *s)
{
  save(p, *s++);
  return s;
}

inline static void
reset(perl_parser *p)
{
  memset(p->tokenbuf, '\0', p->tokenlen);
  p->tokenlen = 0;
}

inline static void
fix(perl_parser *p)
{
  p->tokenbuf[p->tokenlen] = '\0';
}

perl_array
perl_canonicalize_name(perl_parser *p, perl_scalar str)
{
  perl_array components = perl_array_new(p->state);
  perl_scalar temp, prev;
  char buf[PERL_IDENT_MAX];
  int len, total = 0;
  char *pos = NULL;
  char *start;

  start = perl_to_str(str)->str;

  while ((pos = strstr(start, "::")) != NULL) {
    pos += 2; /* skip "::" */
    len = pos - start;
    temp = perl_str_new(p->state, start, len);
    perl_array_push(p->state, components, temp);
    start += len;
    total += len;
  }

  if (pos == NULL) {
    temp = perl_str_new(p->state, start, perl_to_str(str)->fill-total);
    perl_array_push(p->state, components, temp);
  }
  return components;
}

perl_value
find_top_pkg(perl_parser *p, perl_scalar name)
{
  perl_scalar lex;
    // my $lex = $*CURLEX;
    // while $lex {
    //     return $lex.{$name} if $lex.{$name};
    //     my $oid = $lex.<OUTER::>[0] || last;
    //     $lex = $ALL.{$oid};
    // }

  if (perl_to_str(name)->str[perl_to_str(name)->fill-1]  != ':' && perl_to_str(name)->str[perl_to_str(name)->fill-2] != ':') {
    perl_scalar new = perl_str_copy(p->state, name);
    perl_scalar ret = perl_str_cat_cstr(p->state, new, "::", 2);
  }
  return lex;
}

perl_scalar
perl_add_our_name(perl_parser *p, perl_hash stash, perl_scalar n)
{
  perl_array components;
  perl_scalar s = perl_str_cat_cstr(p->state, perl_str_copy(p->state, n), "::", 2);
  perl_hash_store(p->state, stash, s, perl_glob_hash_add(perl_hash_new(p->state)));

  return perl_undef_new();
}

node *
node_sym_new(perl_parser *p, perl_scalar s)  
{
  node_sym *n;

  n = malloc(sizeof(node_sym));
  perl_init_node(p, &n->base, NODE_SYM, 0);
  n->sym = s;
  return (node *)n;
}

node *
node_const_new(perl_parser *p, perl_scalar value)  
{
  node_const *n;

  n = malloc(sizeof(node_const));
  perl_init_node(p, &n->base, NODE_CONST, 0);
  n->value = value;
  return (node *)n;
}

static int 
perl_keyword(char *name, size_t len)
{
  int i;

  for (i = 0; i < sizeof(keywords)/sizeof(keyword); i++) {
    if (strcmp(name, keywords[i].key_name) == 0) {
      return keywords[i].key_id;
    }
  }
  return -1;
}

static char *
scan_qstr(perl_parser *p, char *s, int *val)
{
  reset(p);

  while (1) {
    if (*s == '\'') {
      *val = OP_CONST;
      s++;
      break;
    }
    if (*s == -1) {
      yyerror(p, "unterminated string meets end of file");
      return "";
    }
    save(p, *s);
    s++;
  }
  fix(p);
  return s;
}

static char *
scan_str(perl_parser *p, char *s, int term, int *val)
{
  reset(p);

  while (1) {
    if (*s == term) {
      *val = OP_NULL;
      s++;
      break;
    }
    if (*s == -1) {
      yyerror(p, "unterminated string meets end of file");
      return ""; 
    }
    else if (*s == '\\') {
      s++;
      if (*s == term) {
        save(p, *s);
      } else {
        save(p, *s);
      }
      s++;
      continue;
    } else if (*s == '$') {
      *val = OP_STRINGIFY;
      break;
    }
    save(p, *s);
    s++;
  }
  fix(p);
  return s;
}

#define isDIGIT(c)   (isascii(c) && isdigit(c))

node *
node_variable_new(perl_parser *p, perl_variable *v)
{
  node_variable *n;

  n = malloc(sizeof(node_variable));
  perl_init_node(p, &n->base, NODE_ANYVAR, 0);
  n->variable = v;
  return (node *)n;
}

node *
node_qwlist_new(perl_parser *p, perl_scalar v)
{
  node_value *n;

  n = malloc(sizeof(node_value));
  perl_init_node(p, &n->base, NODE_QWLIST, 0);
  n->value = v;
  return (node *)n;
}

char *
scan_num(perl_parser *p, char *s)
{
  int32_t tryi32;
  double value;
  perl_scalar sv;
  _Bool floatit;
  char *lastub = 0;
  int l = 0;

  reset(p);

  switch (*s) {
    default:
      yyerror(p, "panic: scan_num");
    case '0':
      {
        uint32_t i;
        int32_t shift;
        double n = 0.0;
        unsigned long u = 0;
        _Bool overflowed = false;
        _Bool just_zero  = true;        /* just plain 0 or binary number? */
        static const double nvshift[5] = { 1.0, 2.0, 4.0, 8.0, 16.0 };
        static const char* const bases[5] =
        { "", "binary", "", "octal", "hexadecimal" };
        static const char* const Bases[5] =
        { "", "Binary", "", "Octal", "Hexadecimal" };
        static const char* const maxima[5] =
        { "",
          "0b11111111111111111111111111111111",
          "",
          "037777777777",
          "0xffffffff" };
        const char *base, *Base, *max;

        s++;
        if (*s == 'x' || *s == 'X') {
          shift = 4;
          s++;
          just_zero = false;
        } else if (*s == 'b' || *s == 'B') {
          shift = 1;
          s++;
          just_zero = false;
        } else if (*s == '.' || *s == 'e' || *s == 'E') {
          goto decimal;
        } else {
          shift = 3;
        }
        i = 0;

        base = bases[shift];
        Base = Bases[shift];
        max  = maxima[shift];

        for (;;) {

          unsigned long x, b;

          switch (*s) {
            default:
              goto out;
            case '_':
              s++;
              break;
            case '8': case '9':
              if (shift == 3)
                yyerror(p, "Illegal octal digit");
              /* FALL THROUGH */

            case '2': case '3': case '4':
            case '5': case '6': case '7':
              if (shift == 1)
                yyerror(p, "Illegal binary digit");

            case '0': case '1':
              b = *s & 15;              /* ASCII digit -> value of digit */
              s = save_and_next(p, s);
              goto digit;

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
              if (shift != 4)
                goto out;
              b = (*s & 7) + 9;
              s = save_and_next(p, s);
digit:
              just_zero = false;
              if (!overflowed) {
                x = u << shift; /* make room for the digit */

                if ((x >> shift) != u) {
                  overflowed = true;
                  n = (double) u;
                  yyerror(p, "Integer overflow in number");
                } else {
                  u = x | b;            /* add the digit to the end */
                }
              }
              if (overflowed) {
                n *= nvshift[shift];
                n += (double) b;
              }
              break;
          }
        }

out:
        fix(p);
        if (overflowed) {
          if (n > 4294967295.0)
            //      yywarn(p, "%s number > %s non-portable", Base, max);
            perl_warner(p->state, "%s number > %s non-portable", Base, max);
          sv = perl_num_init(n);
        }
        else {
#if UVSIZE > 4
          if (u > 0xffffffff)
            //yywarn(p, "%s number > %s non-portable", Base, max);
#endif
            sv = perl_int_init(u);
        }
      }
      break;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '.':
decimal:
      floatit = false;
      while (isDIGIT(*s) || *s == '_') {
        if (*s == '_') {
          s++;
        } else {
          s = save_and_next(p, s);
        }
      }
      if (*s == '.') {
        s = save_and_next(p, s);
        if (*s != '.') {
          floatit = true;
          while (isDIGIT(*s) || *s == '_') {
            if (*s == '_')
              s++;
            else
              s = save_and_next(p, s);
          }
        }
      }
      if (*s && strchr("eE", *s)) {
        save(p, 'e');
        s++;
        if (strchr("+-0123456789", *s)) {
          floatit = true;
          if (*s == '+' || *s == '-') {
            s = save_and_next(p, s);
          }
          while (isDIGIT(*s)) {
            s = save_and_next(p, s);
          }
        }
      }
      fix(p);

      value = atof(p->tokenbuf);
      tryi32 = (int32_t)(value);
      if (!floatit && (double)tryi32 == value) {
        sv = perl_int_init(tryi32);
      } else {
        sv = perl_num_init(value);
      }
  }
  perl_node *node = node_const_new(p, sv);
  yylval.opval = node;
  return s;
}

perl_variable *
perl_find_my_name(perl_parser *p, perl_scalar name)
{
  int idx = 0;
  perl_variable *v, *prev;
  node_block *b;

  for (b = to_node_block(p->curblock); b; b = to_node_block(b->outer)) {
    for (v = b->variable; v; prev = v, v = v->next) {
      if (perl_str_eq(p->state, name, v->name)) {
        return v;
      }
      idx++;
    }
  }
  return NULL;
}

static char *
force_word(perl_parser *p, char *start, int token, int check_keyword, int allow_pack)
{
  start = skipspace(p, start);
  char *s = start;
  size_t len = 0;

  if (isIDFIRST_lazy_if(s) ||
      (allow_pack && *s == ':'))
  {
    s = scan_word(p, s, p->tokenbuf, sizeof(p->tokenbuf), allow_pack, &len);
    p->tokenlen += len;
    if (check_keyword && perl_keyword(p->tokenbuf, p->tokenlen)) {
      return start;
    }
    if (token == METHOD) {
      s = skipspace(p, s);
      if (*s == '(')
        p->expect = XTERM;
      else {
        p->expect = XOPERATOR;
      }
    }
    p->nexttoke->nexttokval[p->nexttoke->nexttoken].opval
      = node_sym_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
    force_next(p, token);
  }
  return s;
}

void
force_next(perl_parser *p, int token)
{
  p->lex_state = LEX_KNOWNEXT;
  p->nexttoke->nexttype[p->nexttoke->nexttoken++] = token;
  return;
}

static void
force_ident_maybe_lex(perl_parser *p, char pit)
{
    NEXTVAL_NEXTTOKE.ival = pit;
    force_next(p, 'p');
}

static int
ao(perl_parser *p, int toketype)
{
  if (*p->bufptr == '=') {
    p->bufptr++;
    if (toketype == ANDAND)
      yylval.ival = OP_ANDASSIGN;
    else if (toketype == OROR)
      yylval.ival = OP_ORASSIGN;
    else if (toketype == DORDOR)
      yylval.ival = OP_DORASSIGN;
    toketype = ASSIGNOP;
  }
  return REPORT(toketype);
}

int
perl_sublex_start(perl_parser *p, int type)
{
  if (type == OP_CONST || type == OP_NULL) {
    yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
    return THING;
  } else if (type == OP_STRINGIFY) {
    p->lex_state = LEX_INTERPPUSH;
    yylval.ival = FUNC;
    return FUNC;
  }
  return 0;
}

_Bool
takes_special_variable(perl_parser *p, char *s)
{
  if (*s == '"' || *s == '\\' || *s == ',') {
    return true;
  }
  return false;
}

int
perl_yylex(perl_parser *p)
{
  int retval;
  char *d;
  char *s;
  _Bool bop = false;  
  _Bool bof = false;
  s = p->bufptr;
  p->oldoldbufptr = p->oldbufptr;
  p->oldbufptr = s;

  if (p->pending_ident) {
    int tmp = perl_pending_ident(p);
    return ident(tmp);
  }

  switch (p->lex_state) {
  default:
    break;
  case LEX_KNOWNEXT:
    {
      p->nexttoke->nexttoken--; 
      if (p->nexttoke->nexttoken == 0) {
        p->lex_state = LEX_NORMAL;
      }
      yylval.opval = p->nexttoke->nexttokval[p->nexttoke->nexttoken].opval;
      perl_token_add(p, p->nexttoke->nexttype[p->nexttoke->nexttoken], &yylval);
      return p->nexttoke->nexttype[p->nexttoke->nexttoken];
    }
    break;
  case LEX_INTERPPUSH:
    {
      p->lex_state = LEX_INTERPCONCAT;
      yylval.ival = '(';
      return token('(');
    }
    break;
  case LEX_INTERPCONCAT:
    {
      p->lex_state = LEX_INTERPSTART;
      if (p->tokenlen == 0) {
        break;
      } else {
        p->lex_starts++;
        yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
        return token(THING);
      }
    }
    break;
  case LEX_INTERPSTART:
    {
      if (p->lex_starts) {
        if (*s != '"') {
          p->lex_starts = 0;
          yylval.ival = ADDOP;
          return token(ADDOP);
        }
      }
      int val = OP_CONST;
      s = scan_str(p, s, '"', &val);
      if (val == OP_CONST) {
        p->lex_starts++;
        yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
        return token(THING);
      } else if (val == OP_STRINGIFY) {
        p->lex_starts++;
        break;
      } else if (val == OP_NULL) {
        if (p->tokenlen == 0) {
          p->lex_starts = 0;
          p->lex_state = LEX_NORMAL;
          yylval.ival = ')';
          return token(')');
        } else {
          p->lex_state = LEX_INTERPEND;
          p->lex_starts = 0;
          yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
          return token(THING);
        }
      }
    }
    break;
  case LEX_INTERPEND:
    {
      p->lex_state = LEX_NORMAL;  
      yylval.ival = ')';
      return token(')');
    }
    break;
  }

retry:
  switch (*s) {
  case '\0':    /* NUL */
  case '\004':  /* ^D */
  case '\032':  /* ^Z */
  case -1:    /* end of script. */
    return 0;

  /* white spaces */
  case ' ': case '\t': case '\f': case '\r':
  case '\13':
    s++;
    goto retry;

  case '#':     /* it's a comment */
    s = skip(p, s, '\n');
    goto retry;

  case '\n':
    p->line++;
    s++;
    goto retry;

  case '$':
    reset(p);
    save(p, *s);
    s++;
    if (takes_special_variable(p, s)) {
      save(p, *s);
      yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
      s++;
      return token(WORD);
    }
    p->tokenbuf[0] = '$';
    s = scan_ident(p, s, p->tokenbuf + 1,
        sizeof p->tokenbuf - 1, false);
    fix(p);
    if (*s == '{') {
      p->tokenbuf[0] = '%';
    }
    yylval.ival = '$';
    p->pending_ident = '$';
    return token('$');
  case '@':
    reset(p);
    save(p, *s);
    s++;
    p->tokenbuf[0] = '@';
    s = scan_ident(p, s, p->tokenbuf + 1,
        sizeof p->tokenbuf - 1, false);
    fix(p);
    yylval.ival = '@';
    p->pending_ident = '@';
    return token('@');
  case '%':
    reset(p);
    save(p, *s);
    s++;
    p->tokenbuf[0] = '%';
    s = scan_ident(p, s, p->tokenbuf + 1,
        sizeof p->tokenbuf - 1, false);
    fix(p);
    yylval.ival = '%';
    p->pending_ident = '%';
    return token('%');
  case ';':
    yylval.ival = ';';
    s++;
    return token(';');
  case '&':
    s++;
    if (*s++ == '&') {
      return AOPERATOR(ANDAND);
    }
    s--;
    if (p->expect == XOPERATOR) {
      d = s;
      if (d == s) {
        p->saw_infix_sigil = 1;
        return BAop(bof ? OP_NBIT_AND : OP_BIT_AND);
      }
      else
        return BAop(OP_SBIT_AND);
    }

    p->tokenbuf[0] = '&';
    s = scan_ident(p, s - 1, p->tokenbuf + 1, sizeof p->tokenbuf - 1, true);
    yylval.ival = 0;
    if (p->tokenbuf[1]) {
      force_ident_maybe_lex(p, '&');
    }
    else {
      return PREREF('&');
    }
    return term('&');
  case '|':
    s++;
    if (*s++ == '|') {
      return AOPERATOR(OROR);
    }
    s--;
    d = s;
    return BOop(bof ? s == d ? OP_NBIT_OR : OP_SBIT_OR : OP_BIT_OR);
  case '!':
    s++;
    {
      const char tmp = *s++;
      if (tmp == '=') {
        /* was this !=~ where !~ was meant?
         * warn on m:!=~\s+([/?]|[msy]\W|tr\W): */
        return eop(OP_NE);
      }
      if (tmp == '~')
        return pmop(OP_NOT);
    }
    s--;
    return operator('!');
  case '=':
    s++;
    {
      const char tmp = *s++;
      if (tmp == '=') {
        return eop(OP_EQ);
      }
      if (tmp == '>') {
        return operator(',');
      }
      if (tmp == '~')
        return pmop(OP_MATCH);
      if (tmp && isSPACE(*s) && strchr("+-*/%.^&|<",tmp)) {
        perl_warner(p->state, "Reversed %c= operator",(int)tmp);
      }
      s--;
    }
    if (p->expect == XBLOCK) {
      const char *t = s;
      while (SPACE_OR_TAB(*t) || *t == '\r') {
        t++;
      }
    }
    yylval.ival = 0;
    return operator(ASSIGNOP);
  case '{':
    {
      if (p->expect == XSTATE || p->expect == XBLOCK) {
        yylval.ival = '{';
        s++;
        return token('{');
      }
      yylval.ival = HASHBRACK;
      s++;
      return token(HASHBRACK);
    }
  case '}':
    yylval.ival = '}';
    s++;
    return token('}');
  case '(':
    yylval.ival = '(';
    s++;
    return token('(');
  case ')':
    {
      s++;
      s = skipspace(p, s);
      yylval.ival = ')';
      if (*s == '{') {
        return preblock(')');
      }
      return term(')');
    }
  case '\'':
    {
      int val = OP_CONST;
      s++;
      s = scan_qstr(p, s, &val);
      yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
      return token(THING);
    }
    break;
  case '+':
    s++;
    if (*s == '+') {
      s++;
      return token(POSTINC);
    }
    yylval.ival = '+';
    return token(ADDOP);
  case '*':
    yylval.ival = '*';
    s++;
    return token(MULOP);
  case '-':
    s++;
    if (*s == '>') {
      s++;
            s = skipspace(p, s);
            if (isIDFIRST(*s)) {
        p->saw_arrow = true;
        yylval.ival = ARROW; 
        return token(ARROW);
      }
    } else {
      return token('-');
    }
  case ',':
    yylval.ival = ',';
    s++;
    return token(',');
  case '[':
    yylval.ival = '[';
    s++;
    return token('[');
  case ']':
    yylval.ival = ']';
    s++;
    return token(']');
  case '"':
    {
      int val = OP_CONST;
      s++;
      s = scan_str(p, s, '"', &val);
      int tok = perl_sublex_start(p, val);
      return term(tok);
    }
    break;
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    s = scan_num(p, s);
    return token(THING);
  case '_':
  case 'a': case 'A':
  case 'b': case 'B':
  case 'c': case 'C':
  case 'd': case 'D':
  case 'e': case 'E':
  case 'f': case 'F':
  case 'g': case 'G':
  case 'h': case 'H':
  case 'i': case 'I':
  case 'j': case 'J':
  case 'k': case 'K':
  case 'l': case 'L':
  case 'm': case 'M':
  case 'n': case 'N':
  case 'o': case 'O':
  case 'p': case 'P':
  case 'q': case 'Q':
  case 'r': case 'R':
  case 's': case 'S':
  case 't': case 'T':
  case 'u': case 'U':
  case 'v': case 'V':
  case 'w': case 'W':
            case 'X':
  case 'y': case 'Y':
  case 'z': case 'Z':
    return parse_word(p, s);
  }
  perl_warner(p->state, "unknown token '%c' line = %d\n", *s, p->line);
  return 0;
}

_Bool
find_our_name(perl_parser *p, char *tokenbuf, int tokenlen)
{
  perl_scalar *v;

  v = perl_hash_fetch(p->state, p->curstash,
                      perl_str_new(p->state, tokenbuf, tokenlen),
                      perl_undef_new(),
                      false);
  if (v == NULL) {
    return false;
  } else {
    return true;
  }
}

static char *
force_version(perl_parser *p, char *s, int guessing)
{
    char *d;

    s = skipspace(p, s);

    d = s;

    /* NOTE: The parser sees the package name and the VERSION swapped */
    NEXTVAL_NEXTTOKE.opval = NULL;
    force_next(p, WORD);

    return s;
}

static char *
tokenize_use(perl_parser *p, int is_use, char *s)
{
//  if (p->expect != XSTATE)
    //yyerror(p, "\"%s\" not allowed in expression", is_use ? "use" : "no");

  p->expect = XTERM;
  s = skipspace(p, s);
  if (isDIGIT(*s) || (*s == 'v' && isDIGIT(s[1]))) {
    s = force_version(p, s, true);
    if (*s == ';' || *s == '}'
        || (s = skipspace(p, s), (*s == ';' || *s == '}'))) {
      NEXTVAL_NEXTTOKE.opval = NULL;
      force_next(p, WORD);
    }
    else if (*s == 'v') {
      s = force_word(p, s,WORD,false,true);
      s = force_version(p, s, false);
    }
  }
  else {
    s = force_word(p, s,WORD,false,true);
    s = force_version(p, s, false);
  }
  yylval.ival = is_use;
  return s;
}


_Bool
word_takes_any_delimeter(char *p, int len)
{
  return (len == 1 && strchr("msyq", p[0])) ||
    (len == 2 && (
    (p[0] == 't' && p[1] == 'r') ||
    (p[0] == 'q' && strchr("qwxr", p[1]))));
}

char *
scan_word(perl_parser *p, char *s, char *tokenbuf, size_t destlen, _Bool allow_package, size_t *slp)
{
//  char *dest = p->tokenbuf;
  char *dest = tokenbuf;
  char *d = dest;
  char * const e = d + destlen - 3; /* reserved "::\0" */ 
   
  for (;;) {
    if (d >= e)
      perl_croak(p->state, ident_too_long);
    if (isALNUM(*s)) {
      *d++ = *s++;
    } else if (*s == '\'' && allow_package && isIDFIRST(s[1])) {
      s++;
      if (isIDFIRST(*s)) {
        *d++ = ':';
        *d++ = ':';
        s++;
      }
    } else if (*s == ':' && s[1] == ':' && allow_package && isIDFIRST(s[2])) {
      *d++ = *s++;
      *d++ = *s++;
    } else {
      *d = '\0';
      *slp = d - dest;
      return s;
    }
  }
  return s;
}

static char *
force_strict_version(perl_parser *p, char *s)
{
  node *version = NULL;
  const char *errstr = NULL;

  while (isSPACE(*s)) /* leading whitespace */
    s++;

  /* NOTE: The parser sees the package name and the VERSION swapped */
  //NEXTVAL_NEXTTOKE.opval = version;
  p->nexttoke->nexttokval[p->nexttoke->nexttoken].opval = version;
  force_next(p, WORD);

  return s;
}

int
parse_word(perl_parser *p, char *s)
{
  p->bufptr = s;
  char *d;
  size_t len;

  s = scan_word(p, s, p->tokenbuf, sizeof(p->tokenbuf), false, &len);
  p->tokenlen += len;
  if (*s == ':' && s[1] == ':' && strNE(p->tokenbuf, "CORE"))
    goto just_a_word;

  _Bool anydelim = word_takes_any_delimeter(p->tokenbuf, p->tokenlen);
  if (!anydelim && *s == ':' && s[1] == ':') {
    if (strNE(p->tokenbuf, "CORE")) goto just_a_word;
  }

  d = s;
  while (d < p->bufend && isSPACE(*d))
    d++;    /* no comments skipped here, or s### is misparsed */

  if (*d == '=' && d[1] == '>') {
fat_arrow:
    yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
    return term(WORD);
  }

  int tmp = perl_keyword(p->tokenbuf, p->tokenlen);

  /* Is this a label? */
  if (!anydelim && p->expect == XSTATE
      && d < p->bufend && *d == ':' && *(d + 1) != ':') {
    s = d + 1;
    yylval.pval = perl_saveptr(p->tokenbuf, p->tokenlen+1);
    yylval.pval[p->tokenlen] = '\0';
    //yylval.pval[p->tokenlen+1] = UTF ? 1 : 0;
    return token(LABEL);
  }

  /* Not keyword */
  if (tmp < 0) {
    if (*s != ':' || s[1] != ':') {
        perl_scalar gv;
        // gv = perl_hash_fetch(p->state, p->curstash, p->tokenbuf, false, PERL_GLOB_CODE);
        // if (perl_is_undef(gv) && perl_is_imported(gv)) {
        //   tmp = 0;
        // }
    } else {
      tmp = -tmp;
    }
  }

reserved_word:
  switch (tmp) {

  default:      /* not a keyword */
    just_a_word: {

      int pkgname = 0;
      if (*s == '\'' || (*s == ':' && s[1] == ':')) {
        size_t morelen = 0;
        s = scan_word(p, s, p->tokenbuf, sizeof(p->tokenbuf), true, &morelen);
        if (!morelen)
          perl_croak(p->state, "Bad name after %s%s", p->tokenbuf, *s == '\'' ? "'" : "::");
        len += morelen;
        p->tokenlen += morelen;
        pkgname = 1;
      }

      s = skipspace(p, s);
      if (*s == '(') {
        if (p->saw_arrow) {
          p->saw_arrow = false;
          yylval.opval = node_const_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
          return token(METHOD);
        } else {
          NEXTVAL_NEXTTOKE.opval = node_sym_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
          force_next(p, WORD);
          return token('&');
        }
      } else {
        s = skipspace(p, s);
        if (find_our_name(p, p->tokenbuf, p->tokenlen)) {
          if (*s == '-' && s[1] == '>') {
            yylval.opval = node_sym_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
            return token(WORD);
          }
          p->nexttoke->nexttokval[p->nexttoke->nexttoken].opval
            = node_sym_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
          force_next(p, WORD);
          return token('&');
        } else {
          yylval.opval = node_sym_new(p, perl_str_new(p->state, p->tokenbuf, p->tokenlen));
          return token(WORD);
        }
      }
    }
    break;
  case KEY_local:
    yylval.ival = LOCAL;
    return token(LOCAL);
  case KEY_for:
    yylval.ival = FOR;
    return token(FOR);
  case KEY_elsif:
    yylval.ival = ELSIF;
    return token(ELSIF);
  case KEY_if:
    yylval.ival = IF;
    return token(IF);
  case KEY_else:
    yylval.ival = ELSE;
    return token(ELSE);
  case KEY_package:
    s = force_word(p, s, WORD, false, true);
    s = skipspace(p, s);
    s = force_strict_version(p, s);
    yylval.ival = PACKAGE;
    return token(PACKAGE);
  case KEY_our:
  case KEY_my:
    p->in_my = tmp;
    yylval.ival = MY;
    return term(MY);
  case KEY_sub:
    really_sub:
      s = skipspace(p, s);

      if (isIDFIRST(*s) || *s == '\'' || *s == ':') {
        char tmpbuf[128];
        p->expect = XBLOCK;
        size_t len = 0;
        char *d;
        d = scan_word(p, s, p->tokenbuf, sizeof(p->tokenbuf), true, &len);
        if (strchr(tmpbuf, ':'))
          perl_str_cat_cstr(p->state, p->subname, tmpbuf, len);
        else {
          p->subname = perl_str_copy(p->state, p->curstashname);
          perl_str_cat_cstr(p->state, p->subname,"::",2);
          perl_str_cat_cstr(p->state, p->subname,tmpbuf,len);
        }
        s = force_word(p, s, WORD, false, true);
        s = skipspace(p, s);
      }
      else {
        p->expect = XTERMBLOCK;
      }
      return preblock(SUB);
  case KEY_shift:
    yylval.ival = OP_SHIFT;
    s = skipspace(p, s);
    if (*s == '(') {
      return term(FUNC1);
    } else {
      s = skipspace(p, s);
      if (*s == '(') {
        return term(FUNC1);
      } else {
        return term(UNIOP);
      }
    }
  case KEY_use:
    s = tokenize_use(p, 1, s);
    return token(USE);
  case KEY_print:
    yylval.ival = OP_PRINT; 
    s = skipspace(p, s);
    if (*s == '(') {
      return token(FUNC);
    } else {
      return token(LSTOP);
    }
  case KEY_open:
    yylval.ival = OP_OPEN; 
    s = skipspace(p, s);
    if (*s == '(') {
      return token(FUNC);
    } else {
      return token(LSTOP);
    }
  case KEY_eq:
	    return eop(OP_SEQ);
  case KEY_ne:
	    return eop(OP_SNE);
  }
  return 0;
}

char *
scan_ident(perl_parser *p, char *s, char *dest, ssize_t destlen, int ck_uni)
{
  if (isDIGIT(*s)) {
    while (isDIGIT(*s)) {
      save(p, *s);
      s++;
    }
  } else {
    for (;;) {
      if (isALNUM(*s)) {
        save(p, *s);
        s++;
      } else if (*s == '\'') {
        s++;
        if (isIDFIRST(*s)) {
          save(p, ':');
          save(p, ':');
          s++;
        }
      } else if (*s == ':') {
        s++;
        if (*s == ':') {
          save(p, ':');
          save(p, ':');
          s++;
        }
      } else {
        break;
      }
    }
  }
  return s;
}

int
perl_pending_ident(perl_parser *p)
{
  char pit = p->pending_ident;
  const char *const has_colon = (const char*) memchr (p->tokenbuf, ':', p->tokenlen);
  perl_scalar name = perl_str_new(p->state, p->tokenbuf, p->tokenlen);
  perl_variable *tmp;
  p->pending_ident = 0; 

  if (p->in_my) {
    if (p->in_my == KEY_our) {  /* "our" is merely analogous to "my" */
      if (has_colon) {
        perl_warner(p->state, "No package name allowed for variable %s in \"our\"", p->tokenbuf);
      }
      perl_add_our_name(p, p->curstash, name);
      tmp = perl_add_my_name(p, name);
      yylval.opval = node_variable_new(p, tmp);
      return WORD;
    } else {
      if (has_colon) {
        perl_warner(p->state, "\"my\" variable %s can't be in a package", p->tokenbuf);
      }
      tmp = perl_add_my_name(p, name);
      yylval.opval = node_variable_new(p, tmp);
      return PRIVATEREF;
    }
  }

  if (!has_colon) {
    if (!p->in_my) {
      if ((tmp = perl_find_my_name(p, name)) != NULL) {
        if (tmp->scope == PERL_SCOPE_OUR) {
          yylval.opval = node_variable_new(p, tmp);
          return WORD;
        }
        yylval.opval = node_variable_new(p, tmp);
        return PRIVATEREF;
      }
    }
  }
  if (pit != '&') {
    perl_add_our_name(p, p->curstash, name);
  }
  
  tmp = perl_add_my_name(p, name);
  yylval.opval = node_variable_new(p, tmp);
  return WORD;
}

