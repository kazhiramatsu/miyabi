
#ifndef PERL_OPCODE_H
#define PERL_OPCODE_H

#include "perl.h"
#include "value.h"
#include "config.h"
#include "opcode.h"

typedef enum {
  OP_ENTERSUB,
  OP_ENTER,
  OP_LEAVE,
  OP_GV,
  OP_METHOD,
  OP_CONST,
  OP_COND_EXPR,
  OP_SASSIGN,
  OP_AASSIGN,
  OP_PRINT,
  OP_SHIFT,
  OP_OPEN,
  OP_STRINGIFY,
  OP_NULL,
  OP_BLESS,
} OpCode;

/*     A:B:C:OP = 9: 9: 7: 7        */
/*      A:Bx:OP =    9:16: 7        */
/*        Ax:OP =      25: 7        */
/*   A:Bz:Cz:OP = 9:14: 2: 7        */

//	'Ax' : 25 bits ('A', 'B', and 'C' together)
//	`Bx' : 16 bits (`B' and `C' together)
//	`sBx' : signed Bx
enum OpMode {iABC, iABx, iAsBx, iAx};  /* basic instruction format */


/*
** size and position of opcode arguments.
*/
#define SIZE_C		7
#define SIZE_B		9
#define SIZE_Bx		(SIZE_C + SIZE_B)
#define SIZE_A		8
#define SIZE_Ax		(SIZE_C + SIZE_B + SIZE_A)

#define SIZE_OP		7

#define POS_OP		0
#define POS_A		(POS_OP + SIZE_OP)
#define POS_C		(POS_A + SIZE_A)
#define POS_B		(POS_C + SIZE_C)
#define POS_Bx		POS_C
#define POS_Ax		POS_A

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
#define MAXARG_Ax	((1<<SIZE_Ax)-1)
#else
#define MAXARG_Ax	MAX_INT
#endif

#define MAXARG_A        ((1<<SIZE_A)-1)
#define MAXARG_B        ((1<<SIZE_B)-1)
#define MAXARG_C        ((1<<SIZE_C)-1)


/* creates a mask with `n' 1 bits at position `p' */
#define MASK1(n,p)	((~((~(perl_instruction)0)<<(n)))<<(p))

/* creates a mask with `n' 0 bits at position `p' */
#define MASK0(n,p)	(~MASK1(n,p))

/*
** the following macros help to manipulate instructions
*/

#define GET_OPCODE(i)	(cast(OpCode, ((i)>>POS_OP) & MASK1(SIZE_OP,0)))
#define SET_OPCODE(i,o)	((i) = (((i)&MASK0(SIZE_OP,POS_OP)) | \
		((cast(perl_instruction, o)<<POS_OP)&MASK1(SIZE_OP,POS_OP))))

#define getarg(i,pos,size)	(cast(int, ((i)>>pos) & MASK1(size,0)))
#define setarg(i,v,pos,size)	((i) = (((i)&MASK0(size,pos)) | \
                ((cast(perl_instruction, v)<<pos)&MASK1(size,pos))))

#define GETARG_A(i)	getarg(i, POS_A, SIZE_A)
#define SETARG_A(i,v)	setarg(i, v, POS_A, SIZE_A)

#define GETARG_B(i)	getarg(i, POS_B, SIZE_B)
#define SETARG_B(i,v)	setarg(i, v, POS_B, SIZE_B)

#define GETARG_C(i)	getarg(i, POS_C, SIZE_C)
#define SETARG_C(i,v)	setarg(i, v, POS_C, SIZE_C)

#define GETARG_Bx(i)	getarg(i, POS_Bx, SIZE_Bx)
#define SETARG_Bx(i,v)	setarg(i, v, POS_Bx, SIZE_Bx)

#define GETARG_Ax(i)	getarg(i, POS_Ax, SIZE_Ax)
#define SETARG_Ax(i,v)	setarg(i, v, POS_Ax, SIZE_Ax)

#define GETARG_sBx(i)	(GETARG_Bx(i)-MAXARG_sBx)
#define SETARG_sBx(i,b)	SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))


#define CREATE_ABC(o,a,b,c)	((cast(perl_instruction, o)<<POS_OP) \
			| (cast(perl_instruction, a)<<POS_A) \
			| (cast(perl_instruction, b)<<POS_B) \
			| (cast(perl_instruction, c)<<POS_C))

#define CREATE_ABx(o,a,bc)	((cast(perl_instruction, o)<<POS_OP) \
			| (cast(perl_instruction, a)<<POS_A) \
			| (cast(perl_instruction, bc)<<POS_Bx))

#define CREATE_Ax(o,a)		((cast(perl_instruction, o)<<POS_OP) \
			| (cast(perl_instruction, a)<<POS_Ax))


/*
** Macros to operate RK indices
*/

/* this bit 1 means constant (0 means register) */
#define BITRK		(1 << (SIZE_B - 1))

/* test whether value is a constant */
#define ISK(x)		((x) & BITRK)

/* gets the index of the constant */
#define INDEXK(r)	((int)(r) & ~BITRK)

#define MAXINDEXRK	(BITRK - 1)

/* code a constant index as a RK value */
#define RKASK(x)	((x) | BITRK)


/*
** invalid register that fits in 8 bits
*/
#define NO_REG		MAXARG_A

// typedef enum {
// 	OP_NULL,	/* 0 */
// 	OP_STUB,	/* 1 */
// 	OP_SCALAR,	/* 2 */
// 	OP_PUSHMARK,	/* 3 */
// 	OP_WANTARRAY,	/* 4 */
// 	OP_CONST,	/* 5 */
// 	OP_GVSV,	/* 6 */
// 	OP_GV,		/* 7 */
// 	OP_GELEM,	/* 8 */
// 	OP_PADSV,	/* 9 */
// 	OP_PADAV,	/* 10 */
// 	OP_PADHV,	/* 11 */
// 	OP_PADANY,	/* 12 */
// 	OP_PUSHRE,	/* 13 */
// 	OP_RV2GV,	/* 14 */
// 	OP_RV2SV,	/* 15 */
// 	OP_AV2ARYLEN,	/* 16 */
// 	OP_RV2CV,	/* 17 */
// 	OP_ANONCODE,	/* 18 */
// 	OP_PROTOTYPE,	/* 19 */
// 	OP_REFGEN,	/* 20 */
// 	OP_SREFGEN,	/* 21 */
// 	OP_REF,		/* 22 */
// 	OP_BLESS,	/* 23 */
// 	OP_BACKTICK,	/* 24 */
// 	OP_GLOB,	/* 25 */
// 	OP_READLINE,	/* 26 */
// 	OP_RCATLINE,	/* 27 */
// 	OP_REGCMAYBE,	/* 28 */
// 	OP_REGCOMP,	/* 29 */
// 	OP_MATCH,	/* 30 */
// 	OP_SUBST,	/* 31 */
// 	OP_SUBSTCONT,	/* 32 */
// 	OP_TRANS,	/* 33 */
// 	OP_SASSIGN,	/* 34 */
// 	OP_AASSIGN,	/* 35 */
// 	OP_CHOP,	/* 36 */
// 	OP_SCHOP,	/* 37 */
// 	OP_CHOMP,	/* 38 */
// 	OP_SCHOMP,	/* 39 */
// 	OP_DEFINED,	/* 40 */
// 	OP_UNDEF,	/* 41 */
// 	OP_STUDY,	/* 42 */
// 	OP_POS,		/* 43 */
// 	OP_PREINC,	/* 44 */
// 	OP_I_PREINC,	/* 45 */
// 	OP_PREDEC,	/* 46 */
// 	OP_I_PREDEC,	/* 47 */
// 	OP_POSTINC,	/* 48 */
// 	OP_I_POSTINC,	/* 49 */
// 	OP_POSTDEC,	/* 50 */
// 	OP_I_POSTDEC,	/* 51 */
// 	OP_POW,		/* 52 */
// 	OP_MULTIPLY,	/* 53 */
// 	OP_I_MULTIPLY,	/* 54 */
// 	OP_DIVIDE,	/* 55 */
// 	OP_I_DIVIDE,	/* 56 */
// 	OP_MODULO,	/* 57 */
// 	OP_I_MODULO,	/* 58 */
// 	OP_REPEAT,	/* 59 */
// 	OP_ADD,		/* 60 */
// 	OP_I_ADD,	/* 61 */
// 	OP_SUBTRACT,	/* 62 */
// 	OP_I_SUBTRACT,	/* 63 */
// 	OP_CONCAT,	/* 64 */
// 	OP_STRINGIFY,	/* 65 */
// 	OP_LEFT_SHIFT,	/* 66 */
// 	OP_RIGHT_SHIFT,	/* 67 */
// 	OP_LT,		/* 68 */
// 	OP_I_LT,	/* 69 */
// 	OP_GT,		/* 70 */
// 	OP_I_GT,	/* 71 */
// 	OP_LE,		/* 72 */
// 	OP_I_LE,	/* 73 */
// 	OP_GE,		/* 74 */
// 	OP_I_GE,	/* 75 */
// 	OP_EQ,		/* 76 */
// 	OP_I_EQ,	/* 77 */
// 	OP_NE,		/* 78 */
// 	OP_I_NE,	/* 79 */
// 	OP_NCMP,	/* 80 */
// 	OP_I_NCMP,	/* 81 */
// 	OP_SLT,		/* 82 */
// 	OP_SGT,		/* 83 */
// 	OP_SLE,		/* 84 */
// 	OP_SGE,		/* 85 */
// 	OP_SEQ,		/* 86 */
// 	OP_SNE,		/* 87 */
// 	OP_SCMP,	/* 88 */
// 	OP_BIT_AND,	/* 89 */
// 	OP_BIT_XOR,	/* 90 */
// 	OP_BIT_OR,	/* 91 */
// 	OP_NEGATE,	/* 92 */
// 	OP_I_NEGATE,	/* 93 */
// 	OP_NOT,		/* 94 */
// 	OP_COMPLEMENT,	/* 95 */
// 	OP_ATAN2,	/* 96 */
// 	OP_SIN,		/* 97 */
// 	OP_COS,		/* 98 */
// 	OP_RAND,	/* 99 */
// 	OP_SRAND,	/* 100 */
// 	OP_EXP,		/* 101 */
// 	OP_LOG,		/* 102 */
// 	OP_SQRT,	/* 103 */
// 	OP_INT,		/* 104 */
// 	OP_HEX,		/* 105 */
// 	OP_OCT,		/* 106 */
// 	OP_ABS,		/* 107 */
// 	OP_LENGTH,	/* 108 */
// 	OP_SUBSTR,	/* 109 */
// 	OP_VEC,		/* 110 */
// 	OP_INDEX,	/* 111 */
// 	OP_RINDEX,	/* 112 */
// 	OP_SPRINTF,	/* 113 */
// 	OP_FORMLINE,	/* 114 */
// 	OP_ORD,		/* 115 */
// 	OP_CHR,		/* 116 */
// 	OP_CRYPT,	/* 117 */
// 	OP_UCFIRST,	/* 118 */
// 	OP_LCFIRST,	/* 119 */
// 	OP_UC,		/* 120 */
// 	OP_LC,		/* 121 */
// 	OP_QUOTEMETA,	/* 122 */
// 	OP_RV2AV,	/* 123 */
// 	OP_AELEMFAST,	/* 124 */
// 	OP_AELEM,	/* 125 */
// 	OP_ASLICE,	/* 126 */
// 	OP_EACH,	/* 127 */
// 	OP_VALUES,	/* 128 */
// 	OP_KEYS,	/* 129 */
// 	OP_DELETE,	/* 130 */
// 	OP_EXISTS,	/* 131 */
// 	OP_RV2HV,	/* 132 */
// 	OP_HELEM,	/* 133 */
// 	OP_HSLICE,	/* 134 */
// 	OP_UNPACK,	/* 135 */
// 	OP_PACK,	/* 136 */
// 	OP_SPLIT,	/* 137 */
// 	OP_JOIN,	/* 138 */
// 	OP_LIST,	/* 139 */
// 	OP_LSLICE,	/* 140 */
// 	OP_ANONLIST,	/* 141 */
// 	OP_ANONHASH,	/* 142 */
// 	OP_SPLICE,	/* 143 */
// 	OP_PUSH,	/* 144 */
// 	OP_POP,		/* 145 */
// 	OP_SHIFT,	/* 146 */
// 	OP_UNSHIFT,	/* 147 */
// 	OP_SORT,	/* 148 */
// 	OP_REVERSE,	/* 149 */
// 	OP_GREPSTART,	/* 150 */
// 	OP_GREPWHILE,	/* 151 */
// 	OP_MAPSTART,	/* 152 */
// 	OP_MAPWHILE,	/* 153 */
// 	OP_RANGE,	/* 154 */
// 	OP_FLIP,	/* 155 */
// 	OP_FLOP,	/* 156 */
// 	OP_AND,		/* 157 */
// 	OP_OR,		/* 158 */
// 	OP_XOR,		/* 159 */
// 	OP_COND_EXPR,	/* 160 */
// 	OP_ANDASSIGN,	/* 161 */
// 	OP_ORASSIGN,	/* 162 */
// 	OP_METHOD,	/* 163 */
// 	OP_ENTERSUB,	/* 164 */
// 	OP_LEAVESUB,	/* 165 */
// 	OP_CALLER,	/* 166 */
// 	OP_WARN,	/* 167 */
// 	OP_DIE,		/* 168 */
// 	OP_RESET,	/* 169 */
// 	OP_LINESEQ,	/* 170 */
// 	OP_NEXTSTATE,	/* 171 */
// 	OP_DBSTATE,	/* 172 */
// 	OP_UNSTACK,	/* 173 */
// 	OP_ENTER,	/* 174 */
// 	OP_LEAVE,	/* 175 */
// 	OP_SCOPE,	/* 176 */
// 	OP_ENTERITER,	/* 177 */
// 	OP_ITER,	/* 178 */
// 	OP_ENTERLOOP,	/* 179 */
// 	OP_LEAVELOOP,	/* 180 */
// 	OP_RETURN,	/* 181 */
// 	OP_LAST,	/* 182 */
// 	OP_NEXT,	/* 183 */
// 	OP_REDO,	/* 184 */
// 	OP_DUMP,	/* 185 */
// 	OP_GOTO,	/* 186 */
// 	OP_EXIT,	/* 187 */
// 	OP_OPEN,	/* 188 */
// 	OP_CLOSE,	/* 189 */
// 	OP_PIPE_OP,	/* 190 */
// 	OP_FILENO,	/* 191 */
// 	OP_UMASK,	/* 192 */
// 	OP_BINMODE,	/* 193 */
// 	OP_TIE,		/* 194 */
// 	OP_UNTIE,	/* 195 */
// 	OP_TIED,	/* 196 */
// 	OP_DBMOPEN,	/* 197 */
// 	OP_DBMCLOSE,	/* 198 */
// 	OP_SSELECT,	/* 199 */
// 	OP_SELECT,	/* 200 */
// 	OP_GETC,	/* 201 */
// 	OP_READ,	/* 202 */
// 	OP_ENTERWRITE,	/* 203 */
// 	OP_LEAVEWRITE,	/* 204 */
// 	OP_PRTF,	/* 205 */
// 	OP_PRINT,	/* 206 */
// 	OP_SYSOPEN,	/* 207 */
// 	OP_SYSREAD,	/* 208 */
// 	OP_SYSWRITE,	/* 209 */
// 	OP_SEND,	/* 210 */
// 	OP_RECV,	/* 211 */
// 	OP_EOF,		/* 212 */
// 	OP_TELL,	/* 213 */
// 	OP_SEEK,	/* 214 */
// 	OP_TRUNCATE,	/* 215 */
// 	OP_FCNTL,	/* 216 */
// 	OP_IOCTL,	/* 217 */
// 	OP_FLOCK,	/* 218 */
// 	OP_SOCKET,	/* 219 */
// 	OP_SOCKPAIR,	/* 220 */
// 	OP_BIND,	/* 221 */
// 	OP_CONNECT,	/* 222 */
// 	OP_LISTEN,	/* 223 */
// 	OP_ACCEPT,	/* 224 */
// 	OP_SHUTDOWN,	/* 225 */
// 	OP_GSOCKOPT,	/* 226 */
// 	OP_SSOCKOPT,	/* 227 */
// 	OP_GETSOCKNAME,	/* 228 */
// 	OP_GETPEERNAME,	/* 229 */
// 	OP_LSTAT,	/* 230 */
// 	OP_STAT,	/* 231 */
// 	OP_FTRREAD,	/* 232 */
// 	OP_FTRWRITE,	/* 233 */
// 	OP_FTREXEC,	/* 234 */
// 	OP_FTEREAD,	/* 235 */
// 	OP_FTEWRITE,	/* 236 */
// 	OP_FTEEXEC,	/* 237 */
// 	OP_FTIS,	/* 238 */
// 	OP_FTEOWNED,	/* 239 */
// 	OP_FTROWNED,	/* 240 */
// 	OP_FTZERO,	/* 241 */
// 	OP_FTSIZE,	/* 242 */
// 	OP_FTMTIME,	/* 243 */
// 	OP_FTATIME,	/* 244 */
// 	OP_FTCTIME,	/* 245 */
// 	OP_FTSOCK,	/* 246 */
// 	OP_FTCHR,	/* 247 */
// 	OP_FTBLK,	/* 248 */
// 	OP_FTFILE,	/* 249 */
// 	OP_FTDIR,	/* 250 */
// 	OP_FTPIPE,	/* 251 */
// 	OP_FTLINK,	/* 252 */
// 	OP_FTSUID,	/* 253 */
// 	OP_FTSGID,	/* 254 */
// 	OP_FTSVTX,	/* 255 */
// 	OP_FTTTY,	/* 256 */
// 	OP_FTTEXT,	/* 257 */
// 	OP_FTBINARY,	/* 258 */
// 	OP_CHDIR,	/* 259 */
// 	OP_CHOWN,	/* 260 */
// 	OP_CHROOT,	/* 261 */
// 	OP_UNLINK,	/* 262 */
// 	OP_CHMOD,	/* 263 */
// 	OP_UTIME,	/* 264 */
// 	OP_RENAME,	/* 265 */
// 	OP_LINK,	/* 266 */
// 	OP_SYMLINK,	/* 267 */
// 	OP_READLINK,	/* 268 */
// 	OP_MKDIR,	/* 269 */
// 	OP_RMDIR,	/* 270 */
// 	OP_OPEN_DIR,	/* 271 */
// 	OP_READDIR,	/* 272 */
// 	OP_TELLDIR,	/* 273 */
// 	OP_SEEKDIR,	/* 274 */
// 	OP_REWINDDIR,	/* 275 */
// 	OP_CLOSEDIR,	/* 276 */
// 	OP_FORK,	/* 277 */
// 	OP_WAIT,	/* 278 */
// 	OP_WAITPID,	/* 279 */
// 	OP_SYSTEM,	/* 280 */
// 	OP_EXEC,	/* 281 */
// 	OP_KILL,	/* 282 */
// 	OP_GETPPID,	/* 283 */
// 	OP_GETPGRP,	/* 284 */
// 	OP_SETPGRP,	/* 285 */
// 	OP_GETPRIORITY,	/* 286 */
// 	OP_SETPRIORITY,	/* 287 */
// 	OP_TIME,	/* 288 */
// 	OP_TMS,		/* 289 */
// 	OP_LOCALTIME,	/* 290 */
// 	OP_GMTIME,	/* 291 */
// 	OP_ALARM,	/* 292 */
// 	OP_SLEEP,	/* 293 */
// 	OP_SHMGET,	/* 294 */
// 	OP_SHMCTL,	/* 295 */
// 	OP_SHMREAD,	/* 296 */
// 	OP_SHMWRITE,	/* 297 */
// 	OP_MSGGET,	/* 298 */
// 	OP_MSGCTL,	/* 299 */
// 	OP_MSGSND,	/* 300 */
// 	OP_MSGRCV,	/* 301 */
// 	OP_SEMGET,	/* 302 */
// 	OP_SEMCTL,	/* 303 */
// 	OP_SEMOP,	/* 304 */
// 	OP_REQUIRE,	/* 305 */
// 	OP_DOFILE,	/* 306 */
// 	OP_ENTEREVAL,	/* 307 */
// 	OP_LEAVEEVAL,	/* 308 */
// 	OP_ENTERTRY,	/* 309 */
// 	OP_LEAVETRY,	/* 310 */
// 	OP_GHBYNAME,	/* 311 */
// 	OP_GHBYADDR,	/* 312 */
// 	OP_GHOSTENT,	/* 313 */
// 	OP_GNBYNAME,	/* 314 */
// 	OP_GNBYADDR,	/* 315 */
// 	OP_GNETENT,	/* 316 */
// 	OP_GPBYNAME,	/* 317 */
// 	OP_GPBYNUMBER,	/* 318 */
// 	OP_GPROTOENT,	/* 319 */
// 	OP_GSBYNAME,	/* 320 */
// 	OP_GSBYPORT,	/* 321 */
// 	OP_GSERVENT,	/* 322 */
// 	OP_SHOSTENT,	/* 323 */
// 	OP_SNETENT,	/* 324 */
// 	OP_SPROTOENT,	/* 325 */
// 	OP_SSERVENT,	/* 326 */
// 	OP_EHOSTENT,	/* 327 */
// 	OP_ENETENT,	/* 328 */
// 	OP_EPROTOENT,	/* 329 */
// 	OP_ESERVENT,	/* 330 */
// 	OP_GPWNAM,	/* 331 */
// 	OP_GPWUID,	/* 332 */
// 	OP_GPWENT,	/* 333 */
// 	OP_SPWENT,	/* 334 */
// 	OP_EPWENT,	/* 335 */
// 	OP_GGRNAM,	/* 336 */
// 	OP_GGRGID,	/* 337 */
// 	OP_GGRENT,	/* 338 */
// 	OP_SGRENT,	/* 339 */
// 	OP_EGRENT,	/* 340 */
// 	OP_GETLOGIN,	/* 341 */
// 	OP_SYSCALL,	/* 342 */
// 	OP_max		
// } OpCode;

#endif
