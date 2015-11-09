/*
 * Copyright (C) 1984-2012  Mark Nudelman
 * Modified for use with illumos by Garrett D'Amore.
 * Copyright 2014 Garrett D'Amore <garrett@damore.org>
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Less License, as specified in the README file.
 *
 * For more information, see the README file.
 */

#include "less.h"

#define	WHITESP(c)	((c) == ' ' || (c) == '\t')

char *tags = "tags";

static int total;
static int curseq;

extern int linenums;
extern volatile sig_atomic_t sigs;

enum tag_result {
	TAG_FOUND,
	TAG_NOFILE,
	TAG_NOTAG,
	TAG_NOTYPE,
	TAG_INTR
};

static enum tag_result findctag(char *);
static char *nextctag(void);
static char *prevctag(void);
static off_t ctagsearch(void);

/*
 * The list of tags generated by the last findctag() call.
 */
struct taglist {
	struct tag *tl_first;
	struct tag *tl_last;
};
#define	TAG_END  ((struct tag *)&taglist)
static struct taglist taglist = { TAG_END, TAG_END };
struct tag {
	struct tag *next, *prev; /* List links */
	char *tag_file;		/* Source file containing the tag */
	LINENUM tag_linenum;	/* Appropriate line number in source file */
	char *tag_pattern;	/* Pattern used to find the tag */
	int tag_endline;	/* True if the pattern includes '$' */
};
static struct tag *curtag;

#define	TAG_INS(tp) \
	(tp)->next = TAG_END; \
	(tp)->prev = taglist.tl_last; \
	taglist.tl_last->next = (tp); \
	taglist.tl_last = (tp);

#define	TAG_RM(tp) \
	(tp)->next->prev = (tp)->prev; \
	(tp)->prev->next = (tp)->next;

/*
 * Delete tag structures.
 */
void
cleantags(void)
{
	struct tag *tp;

	/*
	 * Delete any existing tag list.
	 * {{ Ideally, we wouldn't do this until after we know that we
	 *    can load some other tag information. }}
	 */
	while ((tp = taglist.tl_first) != TAG_END) {
		TAG_RM(tp);
		free(tp);
	}
	curtag = NULL;
	total = curseq = 0;
}

/*
 * Create a new tag entry.
 */
static struct tag *
maketagent(char *file, LINENUM linenum, char *pattern, int endline)
{
	struct tag *tp;

	tp = ecalloc(sizeof (struct tag), 1);
	tp->tag_file = estrdup(file);
	tp->tag_linenum = linenum;
	tp->tag_endline = endline;
	if (pattern == NULL)
		tp->tag_pattern = NULL;
	else
		tp->tag_pattern = estrdup(pattern);
	return (tp);
}

/*
 * Find tags in tag file.
 */
void
findtag(char *tag)
{
	enum tag_result result;

	result = findctag(tag);
	switch (result) {
	case TAG_FOUND:
	case TAG_INTR:
		break;
	case TAG_NOFILE:
		error("No tags file", NULL);
		break;
	case TAG_NOTAG:
		error("No such tag in tags file", NULL);
		break;
	case TAG_NOTYPE:
		error("unknown tag type", NULL);
		break;
	}
}

/*
 * Search for a tag.
 */
off_t
tagsearch(void)
{
	if (curtag == NULL)
		return (-1);   /* No tags loaded! */
	if (curtag->tag_linenum != 0)
		return (find_pos(curtag->tag_linenum));
	return (ctagsearch());
}

/*
 * Go to the next tag.
 */
char *
nexttag(int n)
{
	char *tagfile = NULL;

	while (n-- > 0)
		tagfile = nextctag();
	return (tagfile);
}

/*
 * Go to the previous tag.
 */
char *
prevtag(int n)
{
	char *tagfile = NULL;

	while (n-- > 0)
		tagfile = prevctag();
	return (tagfile);
}

/*
 * Return the total number of tags.
 */
int
ntags(void)
{
	return (total);
}

/*
 * Return the sequence number of current tag.
 */
int
curr_tag(void)
{
	return (curseq);
}

/*
 * Find tags in the "tags" file.
 * Sets curtag to the first tag entry.
 */
static enum tag_result
findctag(char *tag)
{
	char *p;
	FILE *f;
	int taglen;
	LINENUM taglinenum;
	char *tagfile;
	char *tagpattern;
	int tagendline;
	int search_char;
	int err;
	char tline[TAGLINE_SIZE];
	struct tag *tp;

	p = shell_unquote(tags);
	f = fopen(p, "r");
	free(p);
	if (f == NULL)
		return (TAG_NOFILE);

	cleantags();
	total = 0;
	taglen = strlen(tag);

	/*
	 * Search the tags file for the desired tag.
	 */
	while (fgets(tline, sizeof (tline), f) != NULL) {
		if (tline[0] == '!')
			/* Skip header of extended format. */
			continue;
		if (strncmp(tag, tline, taglen) != 0 || !WHITESP(tline[taglen]))
			continue;

		/*
		 * Found it.
		 * The line contains the tag, the filename and the
		 * location in the file, separated by white space.
		 * The location is either a decimal line number,
		 * or a search pattern surrounded by a pair of delimiters.
		 * Parse the line and extract these parts.
		 */
		tagpattern = NULL;

		/*
		 * Skip over the whitespace after the tag name.
		 */
		p = skipsp(tline+taglen);
		if (*p == '\0')
			/* File name is missing! */
			continue;

		/*
		 * Save the file name.
		 * Skip over the whitespace after the file name.
		 */
		tagfile = p;
		while (!WHITESP(*p) && *p != '\0')
			p++;
		*p++ = '\0';
		p = skipsp(p);
		if (*p == '\0')
			/* Pattern is missing! */
			continue;

		/*
		 * First see if it is a line number.
		 */
		tagendline = 0;
		taglinenum = getnum(&p, 0, &err);
		if (err) {
			/*
			 * No, it must be a pattern.
			 * Delete the initial "^" (if present) and
			 * the final "$" from the pattern.
			 * Delete any backslash in the pattern.
			 */
			taglinenum = 0;
			search_char = *p++;
			if (*p == '^')
				p++;
			tagpattern = p;
			while (*p != search_char && *p != '\0') {
				if (*p == '\\')
					p++;
				p++;
			}
			tagendline = (p[-1] == '$');
			if (tagendline)
				p--;
			*p = '\0';
		}
		tp = maketagent(tagfile, taglinenum, tagpattern, tagendline);
		TAG_INS(tp);
		total++;
	}
	fclose(f);
	if (total == 0)
		return (TAG_NOTAG);
	curtag = taglist.tl_first;
	curseq = 1;
	return (TAG_FOUND);
}

/*
 * Edit current tagged file.
 */
int
edit_tagfile(void)
{
	if (curtag == NULL)
		return (1);
	return (edit(curtag->tag_file));
}

/*
 * Search for a tag.
 * This is a stripped-down version of search().
 * We don't use search() for several reasons:
 *   -	We don't want to blow away any search string we may have saved.
 *   -	The various regular-expression functions (from different systems:
 *	regcmp vs. re_comp) behave differently in the presence of
 *	parentheses (which are almost always found in a tag).
 */
static off_t
ctagsearch(void)
{
	off_t pos, linepos;
	LINENUM linenum;
	int len;
	char *line;

	pos = ch_zero();
	linenum = find_linenum(pos);

	for (;;) {
		/*
		 * Get lines until we find a matching one or
		 * until we hit end-of-file.
		 */
		if (ABORT_SIGS())
			return (-1);

		/*
		 * Read the next line, and save the
		 * starting position of that line in linepos.
		 */
		linepos = pos;
		pos = forw_raw_line(pos, &line, (int *)NULL);
		if (linenum != 0)
			linenum++;

		if (pos == -1) {
			/*
			 * We hit EOF without a match.
			 */
			error("Tag not found", NULL);
			return (-1);
		}

		/*
		 * If we're using line numbers, we might as well
		 * remember the information we have now (the position
		 * and line number of the current line).
		 */
		if (linenums)
			add_lnum(linenum, pos);

		/*
		 * Test the line to see if we have a match.
		 * Use strncmp because the pattern may be
		 * truncated (in the tags file) if it is too long.
		 * If tagendline is set, make sure we match all
		 * the way to end of line (no extra chars after the match).
		 */
		len = strlen(curtag->tag_pattern);
		if (strncmp(curtag->tag_pattern, line, len) == 0 &&
		    (!curtag->tag_endline || line[len] == '\0' ||
		    line[len] == '\r')) {
			curtag->tag_linenum = find_linenum(linepos);
			break;
		}
	}

	return (linepos);
}

static int circular = 0;	/* 1: circular tag structure */

/*
 * Return the filename required for the next tag in the queue that was setup
 * by findctag().  The next call to ctagsearch() will try to position at the
 * appropriate tag.
 */
static char *
nextctag(void)
{
	struct tag *tp;

	if (curtag == NULL)
		/* No tag loaded */
		return (NULL);

	tp = curtag->next;
	if (tp == TAG_END) {
		if (!circular)
			return (NULL);
		/* Wrapped around to the head of the queue */
		curtag = taglist.tl_first;
		curseq = 1;
	} else {
		curtag = tp;
		curseq++;
	}
	return (curtag->tag_file);
}

/*
 * Return the filename required for the previous ctag in the queue that was
 * setup by findctag().  The next call to ctagsearch() will try to position
 * at the appropriate tag.
 */
static char *
prevctag(void)
{
	struct tag *tp;

	if (curtag == NULL)
		/* No tag loaded */
		return (NULL);

	tp = curtag->prev;
	if (tp == TAG_END) {
		if (!circular)
			return (NULL);
		/* Wrapped around to the tail of the queue */
		curtag = taglist.tl_last;
		curseq = total;
	} else {
		curtag = tp;
		curseq--;
	}
	return (curtag->tag_file);
}
