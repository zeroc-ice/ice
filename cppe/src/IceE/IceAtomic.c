/* ==Id: atomic.S,v 1.4 2001/11/18 00:12:56 davem Exp ==
 * atomic.S: These things are too big to do inline.
 *
 * Copyright (C) 1999 David S. Miller (davem@redhat.com)
 */

/* Recreated as
 * $Id$
 * for use with -Ice-.
 *
 * Build with
 * gcc -shared -o libIceAtomic.so -mcpu=v9 -mtune=v9 IceAtomic.c -fomit-frame-pointer -O2
 * for standalone use, or, better perhaps, as part of libIceUtil.so by
 * gcc         -c                 -mcpu=v9 -mtune=v9 IceAtomic.c -fomit-frame-pointer -O2
 * as part of the src/IceUtil build
 *
/*  Narrative:
 *  This is the linux kernel code for atomic add, atomic subtract on sparc-v9.  In fact,
 *  although the add/sub routines are typed -void- externally, they actually return
 *  the result of their operation.
 *
 *  The exchange_and_add is like add, except that it returns the original
 *  value of the counter (instead of the new one), and for some reason, the
 *  order of its arguments is reversed.
 *
 *  Here is how I think (all three) work.
 *  do {
 * A.   g5  <- value;
 * B.   g7  <- g5 [+/-] delta;
 * C.   if  (value == g5) swap(g7, value);
 * D.   } while (g5 != g7);     // g5 was original value, g7 is swapped from original value
 *                            // if they are not the same, someone changed the memory
 *                            // copy before the swap, so we start over with a new value
 * E.   Synchronize_memory_and_data_cache;
 * F.   return [value+delta | value - delta | value] depending on add/sub/exchange_add.
 *
 *  Notice that step -C.- is an indivisible operation, so everything is good coming
 *  out.  The entire operation can be retried of between -A.- and start of -C.- someone
 *  else changes the counter variable.  The point is that you get a -coherent- result
 *  (which is to say, you don't operate on a stale counter value, and so replace it
 *  with something wrong for everyone).  You might not be operating on the values
 *  at call.  To do that, put global Mutex around the call itself.
 *
 *  WARNINGS:
 *  Do NOT put these in a header file for inlining.  You will get bus errors,
 *  or the results will be wrong if you don't.
 *
 *  Do NOT use these on Sparc 2, 5, 10, 20 etc.  The instructions are
 *  sparc-version 9.
 *
 * These are written for sparc(v9)-linux-gcc.  I have no idea what they will do
 * with Solaris or with other compilers.
 *
 * I do not know if these work in general; I am not a sparc architect.
 *
 * --
 *  Ferris McCormick <fmccor@inforead.com>
 *  06.iii.03
 */

typedef struct {volatile int counter;} atomic_t;

int __atomic_add(int i, atomic_t* v) {
    __asm__ __volatile__ (
"1:	lduw	[%o1], %g5\n"
"	add	%g5, %o0, %g7\n"
"	cas	[%o1], %g5, %g7\n"
"	cmp	%g5, %g7\n"
"	bne,pn	%icc, 1b\n"
"	membar	#StoreLoad | #StoreStore\n"
"	retl\n"
"	add	%g7, %o0, %o0\n"
	);
    return; /* Not Reached */
}

int __atomic_sub(int i, atomic_t *v) {
    __asm__ __volatile__ (
"1:	lduw	[%o1], %g5\n"
"	sub	%g5, %o0, %g7\n"
"	cas	[%o1], %g5, %g7\n"
"	cmp	%g5, %g7\n"
"	bne,pn	%icc, 1b\n"
"	 membar	#StoreLoad | #StoreStore\n"
"	retl\n"
"	 sub	%g7, %o0, %o0\n"
	);
  return; /* Not Reached */
}
int __atomic_exchange_and_add(atomic_t *v, int i) {
#if 0
        int t2;
        t2 = v->counter;
        __atomic_add(i,v);
        return t2;  /* Of course, this is wrong because counter might change
	               after the assignment to t2 but before the add */
#else   /* This is what we actually do */
        __asm__ __volatile__ (
"1:     lduw    [%o0], %g5\n"
"       add     %g5, %o1, %g7\n"
"       cas     [%o0], %g5, %g7\n"
"       cmp     %g5, %g7\n"
"       bne,pn  %icc, 1b\n"
"       membar  #StoreLoad | #StoreStore\n"
"       retl\n"
"       mov     %g7, %o0\n"
        );
        return; /* Not Reached */
#endif
}
