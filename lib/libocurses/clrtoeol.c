/*	$OpenBSD: clrtoeol.c,v 1.3 2005/08/14 17:15:19 espie Exp $ */
/*
 * Copyright (c) 1981, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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

#include "curses.h"

/*
 * wclrtoeol --
 *	Clear up to the end of line.
 */
int
wclrtoeol(win)
	register WINDOW *win;
{
	register int minx, x, y;
	register __LDATA *end, *maxx, *sp;

	y = win->cury;
	x = win->curx;
	if (win->lines[y]->flags & __ISPASTEOL) {
		if (y < win->maxy - 1) {
			y++;
			x = 0;
		} else
			return (OK);
	}
	end = &win->lines[y]->line[win->maxx];
	minx = -1;
	maxx = &win->lines[y]->line[x];
	for (sp = maxx; sp < end; sp++)
		if (sp->ch != ' ' || sp->attr != 0) {
			maxx = sp;
			if (minx == -1)
				minx = sp - win->lines[y]->line;
			sp->ch = ' ';
			sp->attr = 0;
		}
#ifdef DEBUG
	__CTRACE("CLRTOEOL: minx = %d, maxx = %d, firstch = %d, lastch = %d\n",
	    minx, maxx - win->lines[y]->line, *win->lines[y]->firstchp, 
	    *win->lines[y]->lastchp);
#endif
	/* Update firstch and lastch for the line. */
	return (__touchline(win, y, x, win->maxx - 1, 0));
}





