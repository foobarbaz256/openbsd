/*	$OpenBSD: parse.c,v 1.7 2000/09/17 21:28:33 pjanzen Exp $	*/
/*	$NetBSD: parse.c,v 1.3 1995/03/21 15:07:48 cgd Exp $	*/

/*
 * Copyright (c) 1983, 1993
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)parse.c	8.2 (Berkeley) 4/28/95";
#else
static char rcsid[] = "$OpenBSD: parse.c,v 1.7 2000/09/17 21:28:33 pjanzen Exp $";
#endif
#endif /* not lint */

#include "extern.h"

void
wordinit()
{
	struct wlist *w;

	for (w = wlist; w->string; w++)
		install(w);
}

int
hash(s)
	const char   *s;
{
	int     hashval = 0;

	while (*s) {
		hashval += *s++;
		hashval *= HASHMUL;
		hashval &= HASHMASK;
	}
	return hashval;
}

struct wlist *
lookup(s)
	const char   *s;
{
	struct wlist *wp;

	for (wp = hashtab[hash(s)]; wp != NULL; wp = wp->next)
		if (*s == *wp->string && strcmp(s, wp->string) == 0)
			return wp;
	return NULL;
}

void
install(wp)
	struct wlist *wp;
{
	int     hashval;

	if (lookup(wp->string) == NULL) {
		hashval = hash(wp->string);
		wp->next = hashtab[hashval];
		hashtab[hashval] = wp;
	} else
		printf("Multiply defined %s.\n", wp->string);
}

void
parse()
{
	struct wlist *wp;
	int     n;
	int     flag;

	wordnumber = 0;		/* for cypher */
	for (n = 0; n <= wordcount; n++) {
		if ((wp = lookup(words[n])) == NULL) {
			wordvalue[n] = -1;
			wordtype[n] = -1;
		} else {
			wordvalue[n] = wp->value;
			wordtype[n] = wp->article;
		}
	}
	/* Trim "AND AND" which can happen naturally at the end of a
	 * comma-delimited list
	 */
	for (n = 1; n < wordcount; n++)
		if (wordvalue[n - 1] == AND && wordvalue[n] == AND) {
			int i;
			for (i = n + 1; i < wordcount; i++) {
				wordtype[i - 1] = wordtype[i];
				wordvalue[i - 1] = wordvalue[i];
				strlcpy(words[i - 1], words[i], WORDLEN);
			}
			wordcount--;
		}

	/* If there is a sequence (NOUN | OBJECT) AND EVERYTHING
	 * then move all the EVERYTHINGs to the beginning, since that's where
	 * they're expected.  We can't get rid of the NOUNs and OBJECTs in
	 * case they aren't in EVERYTHING (i.e. not here or nonexistant).
	 */
	flag = 1;
	while (flag) {
		flag = 0;
		for (n = 1; n < wordcount; n++)
			if ((wordtype[n - 1] == NOUNS || wordtype[n - 1] == OBJECT) &&
			    wordvalue[n] == AND && wordvalue[n + 1] == EVERYTHING) {
				char tmpword[WORDLEN];
				wordvalue[n + 1] = wordvalue[n - 1];
				wordvalue[n - 1] = EVERYTHING;
				wordtype[n + 1] = wordtype[n - 1];
				wordtype[n - 1] = OBJECT;
				strlcpy(tmpword, words[n - 1], WORDLEN);
				strlcpy(words[n - 1], words[n + 1], WORDLEN);
				strlcpy(words[n + 1], tmpword, WORDLEN);
				flag = 1;
		}
		/* And trim EVERYTHING AND EVERYTHING */
		for (n = 1; n < wordcount; n++)
			if (wordvalue[n - 1] == EVERYTHING &&
			    wordvalue[n] == AND && wordvalue[n + 1] == EVERYTHING) {
				int i;
				for (i = n + 1; i < wordcount; i++) {
					wordtype[i - 1] = wordtype[i + 1];
					wordvalue[i - 1] = wordvalue[i + 1];
					strlcpy(words[i - 1], words[i + 1], WORDLEN);
				}
				wordcount--;
				wordcount--;
				flag = 1;
			}
	}
}
