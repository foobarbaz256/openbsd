/*
 * Copyright (c) 1983, 1993, 2001
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "gprof.h"
#include "cg_arcs.h"
#include "corefile.h"
#include "hist.h"
#include "symtab.h"

    /*
     *        opcode of the `callf' instruction
     */
#define	CALL	(0xc0000000)

void
sparc_find_call (parent, p_lowpc, p_highpc)
     Sym *parent;
     bfd_vma p_lowpc;
     bfd_vma p_highpc;
{
  bfd_vma dest_pc, delta;
  unsigned int *instr;
  Sym *child;

  delta = (bfd_vma) core_text_space - core_text_sect->vma;

  if (core_text_space == 0)
    {
      return;
    }
  if (p_lowpc < s_lowpc)
    {
      p_lowpc = s_lowpc;
    }
  if (p_highpc > s_highpc)
    {
      p_highpc = s_highpc;
    }
  DBG (CALLDEBUG, printf ("[find_call] %s: 0x%lx to 0x%lx\n",
			  parent->name, (unsigned long) p_lowpc,
			  (unsigned long) p_highpc));
  for (instr = (unsigned int *) (((p_lowpc + delta) + 3) &~ 3);
       instr < (unsigned int *) (p_highpc + delta);
       ++instr)
    {
      if ((*instr & CALL))
	{
	  DBG (CALLDEBUG,
	       printf ("[find_call] 0x%lx: callf",
		       (unsigned long) instr - delta));
	  /*
	   * Regular pc relative addressing check that this is the
	   * address of a function.
	   */
	  dest_pc = ((bfd_vma) (instr + (*instr & ~CALL))) - delta;
	  if (dest_pc >= s_lowpc && dest_pc <= s_highpc)
	    {
	      child = sym_lookup (&symtab, dest_pc);
	      DBG (CALLDEBUG,
		   printf ("\tdest_pc=0x%lx, (name=%s, addr=0x%lx)\n",
			   (unsigned long) dest_pc, child->name,
			   (unsigned long) child->addr));
	      if (child->addr == dest_pc)
		{
		  /* a hit:  */
		  arc_add (parent, child, (unsigned long) 0);
		  continue;
		}
	    }
	  /*
	   * Something funny going on.
	   */
	  DBG (CALLDEBUG, printf ("\tbut it's a botch\n"));
	}
    }
}
