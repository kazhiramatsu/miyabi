
#ifndef PERL_OPCODE_H
#define PERL_OPCODE_H

#include "perl.h"
#include "value.h"
#include "config.h"
#include "opcode.h"

/*     A:B:C:OP = 9: 9: 7: 7        */
/*      A:Bx:OP =    9:16: 7        */
/*        Ax:OP =      25: 7        */
/*   A:Bz:Cz:OP = 9:14: 2: 7        */

//  'Ax' : 25 bits ('A', 'B', and 'C' together)
//  `Bx' : 16 bits (`B' and `C' together)
//  `sBx' : signed Bx
enum OpMode {iABC, iABx, iAsBx, iAx};  /* basic instruction format */


/*
** size and position of opcode arguments.
*/
#define SIZE_C    7
#define SIZE_B    9
#define SIZE_Bx   (SIZE_C + SIZE_B)
#define SIZE_A    8
#define SIZE_Ax   (SIZE_C + SIZE_B + SIZE_A)

#define SIZE_OP   7

#define POS_OP    0
#define POS_A   (POS_OP + SIZE_OP)
#define POS_C   (POS_A + SIZE_A)
#define POS_B   (POS_C + SIZE_C)
#define POS_Bx    POS_C
#define POS_Ax    POS_A

/*
** limits for opcode arguments.
** we use (signed) int to manipulate most arguments,
** so they must fit in LUAI_BITSINT-1 bits (-1 for sign)
*/
#if SIZE_Bx < PERL_BITSINT-1
#define MAXARG_Bx        ((1<<SIZE_Bx)-1)
#define MAXARG_sBx        (MAXARG_Bx>>1)         /* `sBx' is signed */
#else
#define MAXARG_Bx        MAX_INT
#define MAXARG_sBx        MAX_INT
#endif

#if SIZE_Ax < PERL_BITSINT-1
#define MAXARG_Ax ((1<<SIZE_Ax)-1)
#else
#define MAXARG_Ax MAX_INT
#endif

#define MAXARG_A        ((1<<SIZE_A)-1)
#define MAXARG_B        ((1<<SIZE_B)-1)
#define MAXARG_C        ((1<<SIZE_C)-1)


/* creates a mask with `n' 1 bits at position `p' */
#define MASK1(n,p)  ((~((~(perl_instruction)0)<<(n)))<<(p))

/* creates a mask with `n' 0 bits at position `p' */
#define MASK0(n,p)  (~MASK1(n,p))

/*
** the following macros help to manipulate instructions
*/

#define GET_OPCODE(i) (cast(OpCode, ((i)>>POS_OP) & MASK1(SIZE_OP,0)))
#define SET_OPCODE(i,o) ((i) = (((i)&MASK0(SIZE_OP,POS_OP)) | \
    ((cast(perl_instruction, o)<<POS_OP)&MASK1(SIZE_OP,POS_OP))))

#define getarg(i,pos,size)  (cast(int, ((i)>>pos) & MASK1(size,0)))
#define setarg(i,v,pos,size)  ((i) = (((i)&MASK0(size,pos)) | \
                ((cast(perl_instruction, v)<<pos)&MASK1(size,pos))))

#define GETARG_A(i) getarg(i, POS_A, SIZE_A)
#define SETARG_A(i,v) setarg(i, v, POS_A, SIZE_A)

#define GETARG_B(i) getarg(i, POS_B, SIZE_B)
#define SETARG_B(i,v) setarg(i, v, POS_B, SIZE_B)

#define GETARG_C(i) getarg(i, POS_C, SIZE_C)
#define SETARG_C(i,v) setarg(i, v, POS_C, SIZE_C)

#define GETARG_Bx(i)  getarg(i, POS_Bx, SIZE_Bx)
#define SETARG_Bx(i,v)  setarg(i, v, POS_Bx, SIZE_Bx)

#define GETARG_Ax(i)  getarg(i, POS_Ax, SIZE_Ax)
#define SETARG_Ax(i,v)  setarg(i, v, POS_Ax, SIZE_Ax)

#define GETARG_sBx(i) (GETARG_Bx(i)-MAXARG_sBx)
#define SETARG_sBx(i,b) SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))


#define CREATE_ABC(o,a,b,c) ((cast(perl_instruction, o)<<POS_OP) \
      | (cast(perl_instruction, a)<<POS_A) \
      | (cast(perl_instruction, b)<<POS_B) \
      | (cast(perl_instruction, c)<<POS_C))

#define CREATE_ABx(o,a,bc)  ((cast(perl_instruction, o)<<POS_OP) \
      | (cast(perl_instruction, a)<<POS_A) \
      | (cast(perl_instruction, bc)<<POS_Bx))

#define CREATE_Ax(o,a)    ((cast(perl_instruction, o)<<POS_OP) \
      | (cast(perl_instruction, a)<<POS_Ax))


/*
** Macros to operate RK indices
*/

/* this bit 1 means constant (0 means register) */
#define BITRK   (1 << (SIZE_B - 1))

/* test whether value is a constant */
#define ISK(x)    ((x) & BITRK)

/* gets the index of the constant */
#define INDEXK(r) ((int)(r) & ~BITRK)

#define MAXINDEXRK  (BITRK - 1)

/* code a constant index as a RK value */
#define RKASK(x)  ((x) | BITRK)


/*
** invalid register that fits in 8 bits
*/
#define NO_REG    MAXARG_A

/*
** R(x) - register
** Kst(x) - constant (in constant table)
** RK(x) == if ISK(x) then Kst(INDEXK(x)) else R(x)
*/


#define RA(i)	(base+GETARG_A(i))
/* to be used after possible stack reallocation */
#define RB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))
#define KBx(i)  \
  (k + (GETARG_Bx(i) != 0 ? GETARG_Bx(i) - 1 : GETARG_Ax(ci->u.p.savedpc++)))


typedef enum {
	OP_NULL		 = 0,
	OP_STUB		 = 1,
	OP_SCALAR	 = 2,
	OP_PUSHMARK	 = 3,
	OP_WANTARRAY	 = 4,
	OP_CONST	 = 5,
	OP_GVSV		 = 6,
	OP_GV		 = 7,
	OP_GELEM	 = 8,
	OP_PADSV	 = 9,
	OP_PADAV	 = 10,
	OP_PADHV	 = 11,
	OP_PADANY	 = 12,
	OP_PUSHRE	 = 13,
	OP_RV2GV	 = 14,
	OP_RV2SV	 = 15,
	OP_AV2ARYLEN	 = 16,
	OP_RV2CV	 = 17,
	OP_ANONCODE	 = 18,
	OP_PROTOTYPE	 = 19,
	OP_REFGEN	 = 20,
	OP_SREFGEN	 = 21,
	OP_REF		 = 22,
	OP_BLESS	 = 23,
	OP_BACKTICK	 = 24,
	OP_GLOB		 = 25,
	OP_READLINE	 = 26,
	OP_RCATLINE	 = 27,
	OP_REGCMAYBE	 = 28,
	OP_REGCRESET	 = 29,
	OP_REGCOMP	 = 30,
	OP_MATCH	 = 31,
	OP_QR		 = 32,
	OP_SUBST	 = 33,
	OP_SUBSTCONT	 = 34,
	OP_TRANS	 = 35,
	OP_TRANSR	 = 36,
	OP_SASSIGN	 = 37,
	OP_AASSIGN	 = 38,
	OP_CHOP		 = 39,
	OP_SCHOP	 = 40,
	OP_CHOMP	 = 41,
	OP_SCHOMP	 = 42,
	OP_DEFINED	 = 43,
	OP_UNDEF	 = 44,
	OP_STUDY	 = 45,
	OP_POS		 = 46,
	OP_PREINC	 = 47,
	OP_I_PREINC	 = 48,
	OP_PREDEC	 = 49,
	OP_I_PREDEC	 = 50,
	OP_POSTINC	 = 51,
	OP_I_POSTINC	 = 52,
	OP_POSTDEC	 = 53,
	OP_I_POSTDEC	 = 54,
	OP_POW		 = 55,
	OP_MULTIPLY	 = 56,
	OP_I_MULTIPLY	 = 57,
	OP_DIVIDE	 = 58,
	OP_I_DIVIDE	 = 59,
	OP_MODULO	 = 60,
	OP_I_MODULO	 = 61,
	OP_REPEAT	 = 62,
	OP_ADD		 = 63,
	OP_I_ADD	 = 64,
	OP_SUBTRACT	 = 65,
	OP_I_SUBTRACT	 = 66,
	OP_CONCAT	 = 67,
	OP_STRINGIFY	 = 68,
	OP_LEFT_SHIFT	 = 69,
	OP_RIGHT_SHIFT	 = 70,
	OP_LT		 = 71,
	OP_I_LT		 = 72,
	OP_GT		 = 73,
	OP_I_GT		 = 74,
	OP_LE		 = 75,
	OP_I_LE		 = 76,
	OP_GE		 = 77,
	OP_I_GE		 = 78,
	OP_EQ		 = 79,
	OP_I_EQ		 = 80,
	OP_NE		 = 81,
	OP_I_NE		 = 82,
	OP_NCMP		 = 83,
	OP_I_NCMP	 = 84,
	OP_SLT		 = 85,
	OP_SGT		 = 86,
	OP_SLE		 = 87,
	OP_SGE		 = 88,
	OP_SEQ		 = 89,
	OP_SNE		 = 90,
	OP_SCMP		 = 91,
	OP_BIT_AND	 = 92,
	OP_BIT_XOR	 = 93,
	OP_BIT_OR	 = 94,
	OP_NBIT_AND	 = 95,
	OP_NBIT_XOR	 = 96,
	OP_NBIT_OR	 = 97,
	OP_SBIT_AND	 = 98,
	OP_SBIT_XOR	 = 99,
	OP_SBIT_OR	 = 100,
	OP_NEGATE	 = 101,
	OP_I_NEGATE	 = 102,
	OP_NOT		 = 103,
	OP_COMPLEMENT	 = 104,
	OP_NCOMPLEMENT	 = 105,
	OP_SCOMPLEMENT	 = 106,
	OP_SMARTMATCH	 = 107,
	OP_ATAN2	 = 108,
	OP_SIN		 = 109,
	OP_COS		 = 110,
	OP_RAND		 = 111,
	OP_SRAND	 = 112,
	OP_EXP		 = 113,
	OP_LOG		 = 114,
	OP_SQRT		 = 115,
	OP_INT		 = 116,
	OP_HEX		 = 117,
	OP_OCT		 = 118,
	OP_ABS		 = 119,
	OP_LENGTH	 = 120,
	OP_SUBSTR	 = 121,
	OP_VEC		 = 122,
	OP_INDEX	 = 123,
	OP_RINDEX	 = 124,
	OP_SPRINTF	 = 125,
	OP_FORMLINE	 = 126,
	OP_ORD		 = 127,
	OP_CHR		 = 128,
	OP_CRYPT	 = 129,
	OP_UCFIRST	 = 130,
	OP_LCFIRST	 = 131,
	OP_UC		 = 132,
	OP_LC		 = 133,
	OP_QUOTEMETA	 = 134,
	OP_RV2AV	 = 135,
	OP_AELEMFAST	 = 136,
	OP_AELEMFAST_LEX = 137,
	OP_AELEM	 = 138,
	OP_ASLICE	 = 139,
	OP_KVASLICE	 = 140,
	OP_AEACH	 = 141,
	OP_AKEYS	 = 142,
	OP_AVALUES	 = 143,
	OP_EACH		 = 144,
	OP_VALUES	 = 145,
	OP_KEYS		 = 146,
	OP_DELETE	 = 147,
	OP_EXISTS	 = 148,
	OP_RV2HV	 = 149,
	OP_HELEM	 = 150,
	OP_HSLICE	 = 151,
	OP_KVHSLICE	 = 152,
	OP_MULTIDEREF	 = 153,
	OP_UNPACK	 = 154,
	OP_PACK		 = 155,
	OP_SPLIT	 = 156,
	OP_JOIN		 = 157,
	OP_LIST		 = 158,
	OP_LSLICE	 = 159,
	OP_ANONLIST	 = 160,
	OP_ANONHASH	 = 161,
	OP_SPLICE	 = 162,
	OP_PUSH		 = 163,
	OP_POP		 = 164,
	OP_SHIFT	 = 165,
	OP_UNSHIFT	 = 166,
	OP_SORT		 = 167,
	OP_REVERSE	 = 168,
	OP_GREPSTART	 = 169,
	OP_GREPWHILE	 = 170,
	OP_MAPSTART	 = 171,
	OP_MAPWHILE	 = 172,
	OP_RANGE	 = 173,
	OP_FLIP		 = 174,
	OP_FLOP		 = 175,
	OP_AND		 = 176,
	OP_OR		 = 177,
	OP_XOR		 = 178,
	OP_DOR		 = 179,
	OP_COND_EXPR	 = 180,
	OP_ANDASSIGN	 = 181,
	OP_ORASSIGN	 = 182,
	OP_DORASSIGN	 = 183,
	OP_METHOD	 = 184,
	OP_ENTERSUB	 = 185,
	OP_LEAVESUB	 = 186,
	OP_LEAVESUBLV	 = 187,
	OP_CALLER	 = 188,
	OP_WARN		 = 189,
	OP_DIE		 = 190,
	OP_RESET	 = 191,
	OP_LINESEQ	 = 192,
	OP_NEXTSTATE	 = 193,
	OP_DBSTATE	 = 194,
	OP_UNSTACK	 = 195,
	OP_ENTER	 = 196,
	OP_LEAVE	 = 197,
	OP_SCOPE	 = 198,
	OP_ENTERITER	 = 199,
	OP_ITER		 = 200,
	OP_ENTERLOOP	 = 201,
	OP_LEAVELOOP	 = 202,
	OP_RETURN	 = 203,
	OP_LAST		 = 204,
	OP_NEXT		 = 205,
	OP_REDO		 = 206,
	OP_DUMP		 = 207,
	OP_GOTO		 = 208,
	OP_EXIT		 = 209,
	OP_METHOD_NAMED	 = 210,
	OP_METHOD_SUPER	 = 211,
	OP_METHOD_REDIR	 = 212,
	OP_METHOD_REDIR_SUPER = 213,
	OP_ENTERGIVEN	 = 214,
	OP_LEAVEGIVEN	 = 215,
	OP_ENTERWHEN	 = 216,
	OP_LEAVEWHEN	 = 217,
	OP_BREAK	 = 218,
	OP_CONTINUE	 = 219,
	OP_OPEN		 = 220,
	OP_CLOSE	 = 221,
	OP_PIPE_OP	 = 222,
	OP_FILENO	 = 223,
	OP_UMASK	 = 224,
	OP_BINMODE	 = 225,
	OP_TIE		 = 226,
	OP_UNTIE	 = 227,
	OP_TIED		 = 228,
	OP_DBMOPEN	 = 229,
	OP_DBMCLOSE	 = 230,
	OP_SSELECT	 = 231,
	OP_SELECT	 = 232,
	OP_GETC		 = 233,
	OP_READ		 = 234,
	OP_ENTERWRITE	 = 235,
	OP_LEAVEWRITE	 = 236,
	OP_PRTF		 = 237,
	OP_PRINT	 = 238,
	OP_SAY		 = 239,
	OP_SYSOPEN	 = 240,
	OP_SYSSEEK	 = 241,
	OP_SYSREAD	 = 242,
	OP_SYSWRITE	 = 243,
	OP_EOF		 = 244,
	OP_TELL		 = 245,
	OP_SEEK		 = 246,
	OP_TRUNCATE	 = 247,
	OP_FCNTL	 = 248,
	OP_IOCTL	 = 249,
	OP_FLOCK	 = 250,
	OP_SEND		 = 251,
	OP_RECV		 = 252,
	OP_SOCKET	 = 253,
	OP_SOCKPAIR	 = 254,
	OP_BIND		 = 255,
	OP_CONNECT	 = 256,
	OP_LISTEN	 = 257,
	OP_ACCEPT	 = 258,
	OP_SHUTDOWN	 = 259,
	OP_GSOCKOPT	 = 260,
	OP_SSOCKOPT	 = 261,
	OP_GETSOCKNAME	 = 262,
	OP_GETPEERNAME	 = 263,
	OP_LSTAT	 = 264,
	OP_STAT		 = 265,
	OP_FTRREAD	 = 266,
	OP_FTRWRITE	 = 267,
	OP_FTREXEC	 = 268,
	OP_FTEREAD	 = 269,
	OP_FTEWRITE	 = 270,
	OP_FTEEXEC	 = 271,
	OP_FTIS		 = 272,
	OP_FTSIZE	 = 273,
	OP_FTMTIME	 = 274,
	OP_FTATIME	 = 275,
	OP_FTCTIME	 = 276,
	OP_FTROWNED	 = 277,
	OP_FTEOWNED	 = 278,
	OP_FTZERO	 = 279,
	OP_FTSOCK	 = 280,
	OP_FTCHR	 = 281,
	OP_FTBLK	 = 282,
	OP_FTFILE	 = 283,
	OP_FTDIR	 = 284,
	OP_FTPIPE	 = 285,
	OP_FTSUID	 = 286,
	OP_FTSGID	 = 287,
	OP_FTSVTX	 = 288,
	OP_FTLINK	 = 289,
	OP_FTTTY	 = 290,
	OP_FTTEXT	 = 291,
	OP_FTBINARY	 = 292,
	OP_CHDIR	 = 293,
	OP_CHOWN	 = 294,
	OP_CHROOT	 = 295,
	OP_UNLINK	 = 296,
	OP_CHMOD	 = 297,
	OP_UTIME	 = 298,
	OP_RENAME	 = 299,
	OP_LINK		 = 300,
	OP_SYMLINK	 = 301,
	OP_READLINK	 = 302,
	OP_MKDIR	 = 303,
	OP_RMDIR	 = 304,
	OP_OPEN_DIR	 = 305,
	OP_READDIR	 = 306,
	OP_TELLDIR	 = 307,
	OP_SEEKDIR	 = 308,
	OP_REWINDDIR	 = 309,
	OP_CLOSEDIR	 = 310,
	OP_FORK		 = 311,
	OP_WAIT		 = 312,
	OP_WAITPID	 = 313,
	OP_SYSTEM	 = 314,
	OP_EXEC		 = 315,
	OP_KILL		 = 316,
	OP_GETPPID	 = 317,
	OP_GETPGRP	 = 318,
	OP_SETPGRP	 = 319,
	OP_GETPRIORITY	 = 320,
	OP_SETPRIORITY	 = 321,
	OP_TIME		 = 322,
	OP_TMS		 = 323,
	OP_LOCALTIME	 = 324,
	OP_GMTIME	 = 325,
	OP_ALARM	 = 326,
	OP_SLEEP	 = 327,
	OP_SHMGET	 = 328,
	OP_SHMCTL	 = 329,
	OP_SHMREAD	 = 330,
	OP_SHMWRITE	 = 331,
	OP_MSGGET	 = 332,
	OP_MSGCTL	 = 333,
	OP_MSGSND	 = 334,
	OP_MSGRCV	 = 335,
	OP_SEMOP	 = 336,
	OP_SEMGET	 = 337,
	OP_SEMCTL	 = 338,
	OP_REQUIRE	 = 339,
	OP_DOFILE	 = 340,
	OP_HINTSEVAL	 = 341,
	OP_ENTEREVAL	 = 342,
	OP_LEAVEEVAL	 = 343,
	OP_ENTERTRY	 = 344,
	OP_LEAVETRY	 = 345,
	OP_GHBYNAME	 = 346,
	OP_GHBYADDR	 = 347,
	OP_GHOSTENT	 = 348,
	OP_GNBYNAME	 = 349,
	OP_GNBYADDR	 = 350,
	OP_GNETENT	 = 351,
	OP_GPBYNAME	 = 352,
	OP_GPBYNUMBER	 = 353,
	OP_GPROTOENT	 = 354,
	OP_GSBYNAME	 = 355,
	OP_GSBYPORT	 = 356,
	OP_GSERVENT	 = 357,
	OP_SHOSTENT	 = 358,
	OP_SNETENT	 = 359,
	OP_SPROTOENT	 = 360,
	OP_SSERVENT	 = 361,
	OP_EHOSTENT	 = 362,
	OP_ENETENT	 = 363,
	OP_EPROTOENT	 = 364,
	OP_ESERVENT	 = 365,
	OP_GPWNAM	 = 366,
	OP_GPWUID	 = 367,
	OP_GPWENT	 = 368,
	OP_SPWENT	 = 369,
	OP_EPWENT	 = 370,
	OP_GGRNAM	 = 371,
	OP_GGRGID	 = 372,
	OP_GGRENT	 = 373,
	OP_SGRENT	 = 374,
	OP_EGRENT	 = 375,
	OP_GETLOGIN	 = 376,
	OP_SYSCALL	 = 377,
	OP_LOCK		 = 378,
	OP_ONCE		 = 379,
	OP_CUSTOM	 = 380,
	OP_COREARGS	 = 381,
	OP_RUNCV	 = 382,
	OP_FC		 = 383,
	OP_PADCV	 = 384,
	OP_INTROCV	 = 385,
	OP_CLONECV	 = 386,
	OP_PADRANGE	 = 387,
	OP_REFASSIGN	 = 388,
	OP_LVREF	 = 389,
	OP_LVREFSLICE	 = 390,
	OP_LVAVREF	 = 391,
	OP_ANONCONST	 = 392,
	OP_max		
} OpCode;

#endif
