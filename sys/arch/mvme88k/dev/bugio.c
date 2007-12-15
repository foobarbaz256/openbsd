/*	$OpenBSD: bugio.c,v 1.18 2007/12/15 19:35:50 miod Exp $ */
/*  Copyright (c) 1998 Steve Murphree, Jr. */

#include <sys/param.h>
#include <sys/systm.h>

#include <machine/asm_macro.h>
#include <machine/bugio.h>
#include <machine/prom.h>

register_t ossr3;
register_t bugsr3;

unsigned long bugvec[32], sysbugvec[32];

void bug_vector(void);
void sysbug_vector(void);

#ifdef MULTIPROCESSOR
#include <sys/lock.h>
__cpu_simple_lock_t bug_lock = __SIMPLELOCK_UNLOCKED;
#define	BUG_LOCK()	__cpu_simple_lock(&bug_lock)
#define	BUG_UNLOCK()	__cpu_simple_unlock(&bug_lock)
#else
#define	BUG_LOCK()	do { } while (0)
#define	BUG_UNLOCK()	do { } while (0)
#endif

#define MVMEPROM_CALL(x)						\
	__asm__ __volatile__ ("or r9,r0," __STRING(x));			\
	__asm__ __volatile__ ("tb0 0,r0,496" :::			\
	    "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",		\
	    "r9", "r10", "r11", "r12", "r13")

void
bug_vector()
{
	unsigned long *vbr;
	int i;

	__asm__ __volatile__ ("ldcr %0, cr7" : "=r" (vbr));
	for (i = 0; i < 32; i++)
		vbr[2 * MVMEPROM_VECTOR + i] = bugvec[i];
}

void
sysbug_vector()
{
	unsigned long *vbr;
	int i;

	__asm__ __volatile__ ("ldcr %0, cr7" : "=r" (vbr));
	for (i = 0; i < 32; i++)
		vbr[2 * MVMEPROM_VECTOR + i] = sysbugvec[i];
}

#define	BUGCTXT()							\
{									\
	BUG_LOCK();							\
	psr = get_psr();						\
	set_psr(psr | PSR_IND);			/* paranoia */		\
	bug_vector();							\
	__asm__ __volatile__ ("ldcr %0, cr20" : "=r" (ossr3));		\
	__asm__ __volatile__ ("stcr %0, cr20" :: "r"(bugsr3));		\
}

#define	OSCTXT()							\
{									\
	__asm__ __volatile__ ("ldcr %0, cr20" : "=r" (bugsr3));		\
	__asm__ __volatile__ ("stcr %0, cr20" :: "r"(ossr3));		\
	sysbug_vector();						\
	set_psr(psr);							\
	BUG_UNLOCK();							\
}

void
bugpcrlf(void)
{
	u_int psr;

	BUGCTXT();
	MVMEPROM_CALL(MVMEPROM_OUTCRLF);
	OSCTXT();
}

void
buginit()
{
	__asm__ __volatile__ ("ldcr %0, cr20" : "=r" (bugsr3));
}

char
buginchr(void)
{
	u_int psr;
	int ret;

	BUGCTXT();
	MVMEPROM_CALL(MVMEPROM_INCHR);
	__asm__ __volatile__ ("or %0,r0,r2" : "=r" (ret));
	OSCTXT();
	return ((char)ret & 0xff);
}

void
bugoutchr(int c)
{
	u_int psr;

	BUGCTXT();
	__asm__ __volatile__ ("or r2,r0,%0" : : "r" (c));
	MVMEPROM_CALL(MVMEPROM_OUTCHR);
	OSCTXT();
}

void
bugreturn(void)
{
	u_int psr;

	BUGCTXT();
	MVMEPROM_CALL(MVMEPROM_EXIT);
	OSCTXT();
}

void
bugbrdid(struct mvmeprom_brdid *id)
{
	u_int psr;
	struct mvmeprom_brdid *ptr;

	BUGCTXT();
	MVMEPROM_CALL(MVMEPROM_GETBRDID);
	__asm__ __volatile__ ("or %0,r0,r2" : "=r" (ptr));
	OSCTXT();

	bcopy(ptr, id, sizeof(struct mvmeprom_brdid));
}

void
bugdiskrd(struct mvmeprom_dskio *dio)
{
	u_int psr;

	BUGCTXT();
	__asm__ __volatile__ ("or r2, r0, %0" : : "r" (dio));
	MVMEPROM_CALL(MVMEPROM_DSKRD);
	OSCTXT();
}

#ifdef MULTIPROCESSOR

/*
 * Ask the BUG to start a particular cpu at our provided address.
 */
int
spin_cpu(cpuid_t cpu, vaddr_t address)
{
	u_int psr;
	int ret;

	BUGCTXT();
	__asm__ __volatile__ ("or r2, r0, %0" : : "r" (cpu));
	__asm__ __volatile__ ("or r3, r0, %0" : : "r" (address));
	MVMEPROM_CALL(MVMEPROM_FORKMPU);
	__asm__ __volatile__ ("or %0,r0,r2" : "=r" (ret));
	OSCTXT();

	return (ret);
}

#endif	/* MULTIPROCESSOR */
