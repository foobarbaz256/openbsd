/* $OpenBSD: ap_fnmatch.c,v 1.6 2011/09/17 15:20:57 stsp Exp $ */

/*
 * Copyright (c) 1989, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
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

/*
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 */

#include "ap_config.h"
#include "fnmatch.h"
#include <string.h>
#include <limits.h>

#define EOS     '\0'

/* Limit of recursion during matching attempts. */
#define __FNM_MAX_RECUR	64

static int __fnmatch(const char *, const char *, int, int);
static const char *rangematch(const char *, int, int);

API_EXPORT(int)
ap_fnmatch(const char *pattern, const char *string, int flags)
{
	int e;

	if (strnlen(pattern, PATH_MAX) == PATH_MAX ||
	    strnlen(string, PATH_MAX) == PATH_MAX)
		return (FNM_NOMATCH);
		
	e = __fnmatch(pattern, string, flags, __FNM_MAX_RECUR);
	if (e == -1)
		e = FNM_NOMATCH;
	return (e);
}

int
__fnmatch(const char *pattern, const char *string, int flags, int recur)
{
	const char *stringstart;
	char c, test;
	int e;

	if (recur-- == 0)
		return (-1);

	for (stringstart = string;;) {
		switch (c = *pattern++) {
		case EOS:
			return (*string == EOS ? 0 : FNM_NOMATCH);
		case '?':
			if (*string == EOS)
				return (FNM_NOMATCH);
			if (*string == '/' && (flags & FNM_PATHNAME))
				return (FNM_NOMATCH);
			if (*string == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				return (FNM_NOMATCH);
			++string;
			break;
		case '*':
			c = *pattern;
			/* Collapse multiple stars. */
			while (c == '*')
				c = *++pattern;

			if (*string == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				return (FNM_NOMATCH);

			/* Optimize for pattern with * at end or before /. */
			if (c == EOS) {
				if (flags & FNM_PATHNAME)
					return (strchr(string, '/') == NULL ? 0 : FNM_NOMATCH);
				else
					return (0);
			}
			else if (c == '/' && flags & FNM_PATHNAME) {
				if ((string = strchr(string, '/')) == NULL)
					return (FNM_NOMATCH);
				break;
			}

			/* General case, use recursion. */
			while ((test = *string) != EOS) {
				e = __fnmatch(pattern, string,
				    flags & ~FNM_PERIOD, recur);
				if (e != FNM_NOMATCH)
					return (e);
				if (test == '/' && flags & FNM_PATHNAME)
					break;
				++string;
			}
			return (FNM_NOMATCH);
		case '[':
			if (*string == EOS)
				return (FNM_NOMATCH);
			if (*string == '/' && flags & FNM_PATHNAME)
				return (FNM_NOMATCH);
			if (*string == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				return (FNM_NOMATCH);
			if ((pattern = rangematch(pattern, *string, flags))
			    == NULL)
				return (FNM_NOMATCH);
			++string;
			break;
		case '\\':
			if (!(flags & FNM_NOESCAPE))
				if ((c = *pattern++) == EOS) {
					c = '\\';
					--pattern;
				}
			/* FALLTHROUGH */
		default:
			if (flags & FNM_CASE_BLIND) {
				if (ap_tolower(c) != ap_tolower(*string))
					return (FNM_NOMATCH);
			}
			else if (c != *string)
				return (FNM_NOMATCH);
			string++;
			break;
		}
	/* NOTREACHED */
	}
}

static const char *
rangematch(const char *pattern, int test, int flags)
{
	int negate, ok;
	char c, c2;

	/*
	* A bracket expression starting with an unquoted circumflex
	* character produces unspecified results (IEEE 1003.2-1992,
	* 3.13.2).  This implementation treats it like '!', for
	* consistency with the regular expression syntax.
	* J.T. Conklin (conklin@ngai.kaleida.com)
	*/
	if ((negate = (*pattern == '!' || *pattern == '^')))
		++pattern;

	for (ok = 0; (c = *pattern++) != ']';) {
		if (c == '\\' && !(flags & FNM_NOESCAPE))
			c = *pattern++;
		if (c == EOS)
			return (NULL);
		if (*pattern == '-' && (c2 = *(pattern + 1)) != EOS && c2
		    != ']') {
			pattern += 2;
			if (c2 == '\\' && !(flags & FNM_NOESCAPE))
				c2 = *pattern++;
			if (c2 == EOS)
				return (NULL);
			if ((c <= test && test <= c2)
			    || ((flags & FNM_CASE_BLIND)
			    && ((ap_tolower(c) <= ap_tolower(test))
			    && (ap_tolower(test) <= ap_tolower(c2)))))
				ok = 1;
		}
		else if ((c == test) || ((flags & FNM_CASE_BLIND)
		    && (ap_tolower(c) == ap_tolower(test))))
			ok = 1;
	}
	return (ok == negate ? NULL : pattern);
}


/* This function is an Apache addition */
/* return non-zero if pattern has any glob chars in it */
API_EXPORT(int)
ap_is_fnmatch(const char *pattern)
{
	int nesting;

	nesting = 0;
	while (*pattern) {
		switch (*pattern) {
		case '?':
		case '*':
			return 1;

		case '\\':
			if (*pattern++ == '\0')
				return 0;
			break;

		case '[':    /* '[' is only a glob if it has a matching ']' */
			++nesting;
			break;

		case ']':
			if (nesting)
				return 1;
			break;
		}
		++pattern;
	}
	return 0;
}
