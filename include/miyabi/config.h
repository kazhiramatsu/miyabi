#ifndef PERL_CONFIG_H
#define PERL_CONFIG_H

#include <limits.h>
#include <stddef.h>


/*
@@ LUAI_BITSINT defines the number of bits in an int.
** CHANGE here if Lua cannot automatically detect the number of bits of
** your machine. Probably you do not need to change this.
*/
/* avoid overflows in comparison */
#if INT_MAX-20 < 32760		/* { */
#define PERL_BITSINT	16
#elif INT_MAX > 2147483640L	/* }{ */
/* int has at least 32 bits */
#define PERL_BITSINT	32
#else				/* }{ */
#error "you must define PERL_BITSINT with number of bits in an integer"
#endif				/* } */

/*
@@ PERL_INT32 is an signed integer with exactly 32 bits.
@@ LUAI_UMEM is an unsigned integer big enough to count the total
@* memory used by Lua.
@@ LUAI_MEM is a signed integer big enough to count the total memory
@* used by Lua.
** CHANGE here if for some weird reason the default definitions are not
** good enough for your machine. Probably you do not need to change
** this.
*/
#if PERL_BITSINT >= 32		/* { */
#define PERL_INT32	int
#define PERL_UMEM	size_t
#define PERL_MEM	ptrdiff_t
#else				/* }{ */
/* 16-bit ints */
#define PERL_INT32	long
#define PERL_UMEM	unsigned long
#define PERL_MEM	long
#endif				/* } */

#endif
