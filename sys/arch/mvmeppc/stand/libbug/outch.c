/*
 * bug routines -- assumes that the necessary sections of memory
 * are preserved.
 */
#include <sys/types.h>
#include <machine/prom.h>

void
mvmeprom_outchr(a)
	char a;
{
	asm volatile ("mr 3, %0" :  :"r" (a));
	MVMEPROM_CALL(MVMEPROM_OUTCHR);
}

