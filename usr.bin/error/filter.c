/*	$OpenBSD: filter.c,v 1.7 2003/06/03 02:56:07 millert Exp $	*/
/*	$NetBSD: filter.c,v 1.3 1995/09/02 06:15:28 jtc Exp $	*/

/*
 * Copyright (c) 1980, 1993
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)filter.c	8.1 (Berkeley) 6/6/93";
#endif
static char rcsid[] = "$OpenBSD: filter.c,v 1.7 2003/06/03 02:56:07 millert Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "pathnames.h"

char	*lint_libs[] = {
	IG_FILE1,
	IG_FILE2,
	IG_FILE3,
	IG_FILE4,
	0
};
int	lexsort();
/*
 *	Read the file ERRORNAME of the names of functions in lint
 *	to ignore complaints about.
 */
void
getignored(auxname)
	char	*auxname;
{
	reg	int	i;
	FILE	*fyle;
	char	inbuffer[256];
	uid_t	uid;
	char	filename[MAXPATHLEN];
	char	*username;
	struct	passwd *passwdentry;

	nignored = 0;
	if (auxname == 0){	/* use the default */
		if ( (username = (char *)getlogin()) == NULL){
			username = "Unknown";
			uid = getuid();
			if ( (passwdentry = (struct passwd *)getpwuid(uid)) == NULL){
				return;
			}
		} else {
			if ( (passwdentry = (struct passwd *)getpwnam(username)) == NULL)
				return;
		}
		strlcpy(filename, passwdentry->pw_dir, sizeof(filename));
		(void)strlcat(filename, ERRORNAME, sizeof(filename));
	} else
		(void)strlcpy(filename, auxname, sizeof filename);
#ifdef FULLDEBUG
	printf("Opening file \"%s\" to read names to ignore.\n",
		filename);
#endif
	if ( (fyle = fopen(filename, "r")) == NULL){
#ifdef FULLDEBUG
		warn("Can't open file \"%s\"", filename);
#endif
		return;
	}
	/*
	 *	Make the first pass through the file, counting lines
	 */
	for (nignored = 0;
	    fgets(inbuffer, sizeof(inbuffer)-1, fyle) != NULL; nignored++)
		continue;
	names_ignored = (char **)Calloc(nignored+1, sizeof (char *));
	fclose(fyle);
	if (freopen(filename, "r", fyle) == NULL){
#ifdef FULLDEBUG
		warn("Failure to open \"%s\" for second read.", filename);
#endif
		nignored = 0;
		return;
	}
	for (i=0; i < nignored &&
	    (fgets (inbuffer, sizeof(inbuffer)-1, fyle) != NULL); i++){
		names_ignored[i] = strsave(inbuffer);
		(void)substitute(names_ignored[i], '\n', '\0');
	}
	qsort(names_ignored, nignored, sizeof *names_ignored, lexsort);
#ifdef FULLDEBUG
	printf("Names to ignore follow.\n");
	for (i=0; i < nignored; i++){
		printf("\tIgnore: %s\n", names_ignored[i]);
	}
#endif
}

int lexsort(cpp1, cpp2)
	char	**cpp1, **cpp2;
{
	return(strcmp(*cpp1, *cpp2));
}

int search_ignore(key)
	char	*key;
{
	reg	int	ub, lb;
	reg	int	halfway;
		int	order;

	if (nignored == 0)
		return(-1);
	for(lb = 0, ub = nignored - 1; ub >= lb; ){
		halfway = (ub + lb)/2;
		if ( (order = strcmp(key, names_ignored[halfway])) == 0)
			return(halfway);
		if (order < 0)	/*key is less than probe, throw away above*/
			ub = halfway - 1;
		 else
			lb = halfway + 1;
	}
	return(-1);
}

/*
 *	Tell if the error text is to be ignored.
 *	The error must have been canonicalized, with
 *	the file name the zeroth entry in the errorv,
 *	and the linenumber the second.
 *	Return the new categorization of the error class.
 */
Errorclass
discardit(errorp)
	Eptr	errorp;
{
		int	language;
	reg	int	i;
	Errorclass	errorclass = errorp->error_e_class;

	switch(errorclass){
		case	C_SYNC:
		case	C_NONSPEC:
		case	C_UNKNOWN:	return(errorclass);
		default:	;
	}
	if(errorp->error_lgtext < 2){
		return(C_NONSPEC);
	}
	language = errorp->error_language;
	if(language == INLINT){
		if (errorclass != C_NONSPEC){	/* no file */
			for(i=0; lint_libs[i] != 0; i++){
				if (strcmp(errorp->error_text[0], lint_libs[i]) == 0){
					return(C_DISCARD);
				}
			}
		}
		/* check if the argument to the error message is to be ignored*/
		if (ispunct(lastchar(errorp->error_text[2])))
			clob_last(errorp->error_text[2], '\0');
		if (search_ignore(errorp->error_text[errorclass == C_NONSPEC ? 0 : 2]) >= 0){
			return(C_NULLED);
		}
	}
	return(errorclass);
}
