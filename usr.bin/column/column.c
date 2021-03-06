/*	$OpenBSD: column.c,v 1.23 2016/03/17 05:27:10 bentley Exp $	*/
/*	$NetBSD: column.c,v 1.4 1995/09/02 05:53:03 jtc Exp $	*/

/*
 * Copyright (c) 1989, 1993, 1994
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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void  c_columnate(void);
void *ereallocarray(void *, size_t, size_t);
void *ecalloc(size_t, size_t);
void  input(FILE *);
void  maketbl(void);
void  print(void);
void  r_columnate(void);
void  usage(void);

int termwidth;			/* default terminal width */

int entries;			/* number of records */
int eval;			/* exit value */
int maxlength;			/* longest record */
char **list;			/* array of pointers to records */
char *separator = "\t ";	/* field separator for table option */

int
main(int argc, char *argv[])
{
	struct winsize win;
	FILE *fp;
	int ch, tflag, xflag;
	char *p;
	const char *errstr;

	termwidth = 0;
	if ((p = getenv("COLUMNS")) != NULL)
		termwidth = strtonum(p, 1, INT_MAX, NULL);
	if (termwidth == 0 && ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == 0 &&
	    win.ws_col > 0)
		termwidth = win.ws_col;
	if (termwidth == 0)
		termwidth = 80;

	if (pledge("stdio rpath", NULL) == -1)
		err(1, "pledge");

	tflag = xflag = 0;
	while ((ch = getopt(argc, argv, "c:s:tx")) != -1)
		switch(ch) {
		case 'c':
			termwidth = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(1, "%s: %s", errstr, optarg);
			break;
		case 's':
			separator = optarg;
			break;
		case 't':
			tflag = 1;
			break;
		case 'x':
			xflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!*argv) {
		input(stdin);
	} else {
		for (; *argv; ++argv) {
			if ((fp = fopen(*argv, "r"))) {
				input(fp);
				(void)fclose(fp);
			} else {
				warn("%s", *argv);
				eval = 1;
			}
		}
	}

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (!entries)
		exit(eval);

	if (tflag)
		maketbl();
	else if (maxlength >= termwidth)
		print();
	else if (xflag)
		c_columnate();
	else
		r_columnate();
	exit(eval);
}

#define	TAB	8
void
c_columnate(void)
{
	int chcnt, col, cnt, endcol, numcols;
	char **lp;

	maxlength = (maxlength + TAB) & ~(TAB - 1);
	numcols = termwidth / maxlength;
	endcol = maxlength;
	for (chcnt = col = 0, lp = list;; ++lp) {
		chcnt += printf("%s", *lp);
		if (!--entries)
			break;
		if (++col == numcols) {
			chcnt = col = 0;
			endcol = maxlength;
			putchar('\n');
		} else {
			while ((cnt = ((chcnt + TAB) & ~(TAB - 1))) <= endcol) {
				(void)putchar('\t');
				chcnt = cnt;
			}
			endcol += maxlength;
		}
	}
	if (chcnt)
		putchar('\n');
}

void
r_columnate(void)
{
	int base, chcnt, cnt, col, endcol, numcols, numrows, row;

	maxlength = (maxlength + TAB) & ~(TAB - 1);
	numcols = termwidth / maxlength;
	if (numcols == 0)
		numcols = 1;
	numrows = entries / numcols;
	if (entries % numcols)
		++numrows;

	for (row = 0; row < numrows; ++row) {
		endcol = maxlength;
		for (base = row, chcnt = col = 0; col < numcols; ++col) {
			chcnt += printf("%s", list[base]);
			if ((base += numrows) >= entries)
				break;
			while ((cnt = ((chcnt + TAB) & ~(TAB - 1))) <= endcol) {
				(void)putchar('\t');
				chcnt = cnt;
			}
			endcol += maxlength;
		}
		putchar('\n');
	}
}

void
print(void)
{
	int cnt;
	char **lp;

	for (cnt = entries, lp = list; cnt--; ++lp)
		(void)printf("%s\n", *lp);
}

typedef struct _tbl {
	char **list;
	int cols, *len;
} TBL;
#define	DEFCOLS	25

void
maketbl(void)
{
	TBL *t;
	int coloff, cnt;
	char *p, **lp;
	int *lens, maxcols = DEFCOLS;
	TBL *tbl;
	char **cols;

	t = tbl = ecalloc(entries, sizeof(TBL));
	cols = ereallocarray(NULL, maxcols, sizeof(char *));
	lens = ecalloc(maxcols, sizeof(int));
	for (cnt = 0, lp = list; cnt < entries; ++cnt, ++lp, ++t) {
		for (coloff = 0, p = *lp; (cols[coloff] = strtok(p, separator));
		    p = NULL)
			if (++coloff == maxcols) {
				maxcols += DEFCOLS;
				cols = ereallocarray(cols, maxcols, 
				    sizeof(char *));
				lens = ereallocarray(lens, maxcols,
				    sizeof(int));
				memset(lens + coloff, 0, DEFCOLS * sizeof(int));
			}
		if (coloff == 0)
			continue;
		t->list = ecalloc(coloff, sizeof(char *));
		t->len = ecalloc(coloff, sizeof(int));
		for (t->cols = coloff; --coloff >= 0;) {
			t->list[coloff] = cols[coloff];
			t->len[coloff] = strlen(cols[coloff]);
			if (t->len[coloff] > lens[coloff])
				lens[coloff] = t->len[coloff];
		}
	}
	for (cnt = 0, t = tbl; cnt < entries; ++cnt, ++t) {
		if (t->cols > 0) {
			for (coloff = 0; coloff < t->cols - 1; ++coloff)
				(void)printf("%s%*s", t->list[coloff],
				    lens[coloff] - t->len[coloff] + 2, " ");
			(void)printf("%s\n", t->list[coloff]);
		}
	}
	free(tbl);
	free(lens);
	free(cols);
}

#define	DEFNUM		1000
#define	MAXLINELEN	(LINE_MAX + 1)

void
input(FILE *fp)
{
	static size_t maxentry = DEFNUM;
	int len;
	char *p, buf[MAXLINELEN];

	if (!list)
		list = ecalloc(maxentry, sizeof(char *));
	while (fgets(buf, MAXLINELEN, fp)) {
		for (p = buf; isspace((unsigned char)*p); ++p);
		if (!*p)
			continue;
		if (!(p = strchr(p, '\n'))) {
			warnx("line too long");
			eval = 1;
			continue;
		}
		*p = '\0';
		len = p - buf;
		if (maxlength < len)
			maxlength = len;
		if (entries == maxentry) {
			maxentry += DEFNUM;
			list = ereallocarray(list, maxentry, sizeof(char *));
			memset(list + entries, 0, DEFNUM * sizeof(char *));
		}
		if (!(list[entries++] = strdup(buf)))
			err(1, NULL);
	}
}

void *
ereallocarray(void *oldp, size_t sz1, size_t sz2)
{
	void *p;

	if (!(p = reallocarray(oldp, sz1, sz2)))
		err(1, NULL);
	return (p);
}

void *
ecalloc(size_t sz1, size_t sz2)
{
	void *p;

	if (!(p = calloc(sz1, sz2)))
		err(1, NULL);
	return (p);
}

void
usage(void)
{

	(void)fprintf(stderr,
	    "usage: column [-tx] [-c columns] [-s sep] [file ...]\n");
	exit(1);
}
