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

/*
 * Process command line options.
 *
 * Each option is a single letter which controls a program variable.
 * The options have defaults which may be changed via
 * the command line option, toggled via the "-" command,
 * or queried via the "_" command.
 */

#include "less.h"
#include "option.h"

static struct loption *pendopt;
int plusoption = FALSE;

static char *optstring(char *, char **, char *, char *);
static int flip_triple(int, int);

extern int screen_trashed;
extern int less_is_more;
extern int quit_at_eof;
extern char *every_first_cmd;
extern int opt_use_backslash;

/*
 * Return a printable description of an option.
 */
static char *
opt_desc(struct loption *o)
{
	static char buf[OPTNAME_MAX + 10];
	if (o->oletter == OLETTER_NONE)
		(void) snprintf(buf, sizeof (buf), "--%s", o->onames->oname);
	else
		(void) snprintf(buf, sizeof (buf), "-%c (--%s)",
		    o->oletter, o->onames->oname);
	return (buf);
}

/*
 * Return a string suitable for printing as the "name" of an option.
 * For example, if the option letter is 'x', just return "-x".
 */
char *
propt(int c)
{
	static char buf[8];

	(void) snprintf(buf, sizeof (buf), "-%s", prchar(c));
	return (buf);
}

/*
 * Scan an argument (either from the command line or from the
 * LESS environment variable) and process it.
 */
void
scan_option(char *s)
{
	struct loption *o;
	int optc;
	char *optname;
	char *printopt;
	char *str;
	int set_default;
	int lc;
	int err;
	int moreopt;
	PARG parg;

	if (s == NULL)
		return;

	/*
	 * If we have a pending option which requires an argument,
	 * handle it now.
	 * This happens if the previous option was, for example, "-P"
	 * without a following string.  In that case, the current
	 * option is simply the argument for the previous option.
	 */
	if (pendopt != NULL) {
		switch (pendopt->otype & OTYPE) {
		case STRING:
			(*pendopt->ofunc)(INIT, s);
			break;
		case NUMBER:
			printopt = opt_desc(pendopt);
			*(pendopt->ovar) = getnum(&s, printopt, NULL);
			break;
		}
		pendopt = NULL;
		return;
	}

	set_default = FALSE;
	optname = NULL;
	moreopt = 0;
	o = NULL;

	while (*s != '\0') {
		/*
		 * Check some special cases first.
		 */
		switch (optc = *s++) {
		case ' ':
		case '\t':
		case END_OPTION_STRING:
			continue;
		case '-':
			/*
			 * "--" indicates an option name instead of a letter.
			 */
			if (*s == '-') {
				if (!less_is_more) {
					optname = ++s;
				}
				break;
			}
			/*
			 * "-+" means set these options back to their defaults.
			 * (They may have been set otherwise by previous
			 * options.)
			 */
			if (!less_is_more) {
				set_default = (*s == '+');
				if (set_default)
					s++;
			}
			continue;
		case '+':
			/*
			 * An option prefixed by a "+" is ungotten, so
			 * that it is interpreted as less commands
			 * processed at the start of the first input file.
			 * "++" means process the commands at the start of
			 * EVERY input file.
			 */
			plusoption = TRUE;
			s = optstring(s, &str, propt('+'), NULL);
			if (s == NULL)
				return;
			if (*str == '+')
				every_first_cmd = estrdup(str+1);
			else
				ungetsc(str);
			free(str);
			continue;
		case '0':  case '1':  case '2':  case '3':  case '4':
		case '5':  case '6':  case '7':  case '8':  case '9':
			/*
			 * Special "more" compatibility form "-<number>"
			 * instead of -z<number> to set the scrolling
			 * window size.
			 */
			s--;
			optc = 'z';
			moreopt = 1;
			break;
		case 'n':
			if (less_is_more) {
				moreopt = 1;
				optc = 'z';
			}
			break;
		case 'i':
			if (less_is_more) {
				moreopt = 1;
				optc = 'I';
			}
			break;
		case 'u':
			if (less_is_more) {
				moreopt = 1;
				optc = 'U';
			}
			break;
		case 'e':
			if (less_is_more) {
				moreopt = 1;
				optc = 'E';
			}
			break;
		case 'h':
			if (less_is_more) {
				moreopt = 1;
				optc = '?';
			}
			break;
		case 'd':
			if (less_is_more) {
				moreopt = 1;
				optc = 'M';
			}
			break;
		}

		/*
		 * Not a special case.
		 * Look up the option letter in the option table.
		 */
		err = 0;
		if (optname == NULL) {
			printopt = propt(optc);
			lc = islower(optc);
			o = findopt(optc);
			if (less_is_more && (!moreopt) && (o != NULL) &&
			    ((o->otype & MORE_OK) == 0)) {
				o = NULL;
			}
		} else {
			printopt = optname;
			lc = islower(optname[0]);
			o = findopt_name(&optname, NULL, &err);
			s = optname;
			optname = NULL;
			switch (*s) {
			case ' ':	/* name matches exactly */
			case '\0':
				break;

			case '=':	/* name followed by "=value" */
				if (o != NULL &&
				    (o->otype & OTYPE) != STRING &&
				    (o->otype & OTYPE) != NUMBER) {
					parg.p_string = printopt;
					error("The %s option should not be "
					    "followed by =", &parg);
					return;
				}
				s++;
				break;
			default:	/* name longer than option, bad */
				o = NULL;
			}
		}
		if (o == NULL) {
			parg.p_string = printopt;
			if (less_is_more) {
				error("Illegal option %s (more -h for help)",
				    &parg);
			} else if (err == OPT_AMBIG) {
				error("%s is an ambiguous abbreviation "
				    "(\"less --help\" for help)", &parg);
			} else {
				error("There is no %s option "
				    "(\"less --help\" for help)", &parg);
			}
			return;
		}

		str = NULL;
		switch (o->otype & OTYPE) {
		case BOOL:
			if (set_default)
				*(o->ovar) = o->odefault;
			else
				*(o->ovar) = ! o->odefault;
			break;
		case TRIPLE:
			if (set_default)
				*(o->ovar) = o->odefault;
			else
				*(o->ovar) = flip_triple(o->odefault, lc);
			break;
		case STRING:
			if (*s == '\0') {
				/*
				 * Set pendopt and return.
				 * We will get the string next time
				 * scan_option is called.
				 */
				pendopt = o;
				return;
			}
			/*
			 * Don't do anything here.
			 * All processing of STRING options is done by
			 * the handling function.
			 */
			while (*s == ' ')
				s++;
			s = optstring(s, &str, printopt, o->odesc[1]);
			if (s == NULL)
				return;
			break;
		case NUMBER:
			if (*s == '\0') {
				pendopt = o;
				return;
			}
			*(o->ovar) = getnum(&s, printopt, NULL);
			break;
		}
		/*
		 * If the option has a handling function, call it.
		 */
		if (o->ofunc != NULL)
			(*o->ofunc)(INIT, str);
		free(str);
	}
}

/*
 * Toggle command line flags from within the program.
 * Used by the "-" and "_" commands.
 * how_toggle may be:
 *	OPT_NO_TOGGLE	just report the current setting, without changing it.
 *	OPT_TOGGLE	invert the current setting
 *	OPT_UNSET	set to the default value
 *	OPT_SET		set to the inverse of the default value
 */
void
toggle_option(struct loption *o, int lower, char *s, int how_toggle)
{
	int num;
	int no_prompt;
	int err;
	PARG parg;

	no_prompt = (how_toggle & OPT_NO_PROMPT);
	how_toggle &= ~OPT_NO_PROMPT;

	if (o == NULL) {
		error("No such option", NULL);
		return;
	}

	if (how_toggle == OPT_TOGGLE && (o->otype & NO_TOGGLE)) {
		parg.p_string = opt_desc(o);
		error("Cannot change the %s option", &parg);
		return;
	}

	if (how_toggle == OPT_NO_TOGGLE && (o->otype & NO_QUERY)) {
		parg.p_string = opt_desc(o);
		error("Cannot query the %s option", &parg);
		return;
	}

	/*
	 * Check for something which appears to be a do_toggle
	 * (because the "-" command was used), but really is not.
	 * This could be a string option with no string, or
	 * a number option with no number.
	 */
	switch (o->otype & OTYPE) {
	case STRING:
	case NUMBER:
		if (how_toggle == OPT_TOGGLE && *s == '\0')
			how_toggle = OPT_NO_TOGGLE;
		break;
	}

	if (how_toggle != OPT_NO_TOGGLE && (o->otype & HL_REPAINT))
		repaint_hilite(0);

	/*
	 * Now actually toggle (change) the variable.
	 */
	if (how_toggle != OPT_NO_TOGGLE) {
		switch (o->otype & OTYPE) {
		case BOOL:
			/*
			 * Boolean.
			 */
			switch (how_toggle) {
			case OPT_TOGGLE:
				*(o->ovar) = ! *(o->ovar);
				break;
			case OPT_UNSET:
				*(o->ovar) = o->odefault;
				break;
			case OPT_SET:
				*(o->ovar) = ! o->odefault;
				break;
			}
			break;
		case TRIPLE:
			/*
			 * Triple:
			 *	If user gave the lower case letter, then switch
			 *	to 1 unless already 1, in which case make it 0.
			 *	If user gave the upper case letter, then switch
			 *	to 2 unless already 2, in which case make it 0.
			 */
			switch (how_toggle) {
			case OPT_TOGGLE:
				*(o->ovar) = flip_triple(*(o->ovar), lower);
				break;
			case OPT_UNSET:
				*(o->ovar) = o->odefault;
				break;
			case OPT_SET:
				*(o->ovar) = flip_triple(o->odefault, lower);
				break;
			}
			break;
		case STRING:
			/*
			 * String: don't do anything here.
			 *	The handling function will do everything.
			 */
			switch (how_toggle) {
			case OPT_SET:
			case OPT_UNSET:
				error("Cannot use \"-+\" or \"--\" "
				    "for a string option", NULL);
				return;
			}
			break;
		case NUMBER:
			/*
			 * Number: set the variable to the given number.
			 */
			switch (how_toggle) {
			case OPT_TOGGLE:
				num = getnum(&s, NULL, &err);
				if (!err)
					*(o->ovar) = num;
				break;
			case OPT_UNSET:
				*(o->ovar) = o->odefault;
				break;
			case OPT_SET:
				error("Can't use \"-!\" for a numeric option",
				    NULL);
				return;
			}
			break;
		}
	}

	/*
	 * Call the handling function for any special action
	 * specific to this option.
	 */
	if (o->ofunc != NULL)
		(*o->ofunc)((how_toggle == OPT_NO_TOGGLE) ? QUERY : TOGGLE, s);

	if (how_toggle != OPT_NO_TOGGLE && (o->otype & HL_REPAINT))
		chg_hilite();

	if (!no_prompt) {
		/*
		 * Print a message describing the new setting.
		 */
		switch (o->otype & OTYPE) {
		case BOOL:
		case TRIPLE:
			/*
			 * Print the odesc message.
			 */
			error(o->odesc[*(o->ovar)], NULL);
			break;
		case NUMBER:
			/*
			 * The message is in odesc[1] and has a %d for
			 * the value of the variable.
			 */
			parg.p_int = *(o->ovar);
			error(o->odesc[1], &parg);
			break;
		case STRING:
			/*
			 * Message was already printed by the handling function.
			 */
			break;
		}
	}

	if (how_toggle != OPT_NO_TOGGLE && (o->otype & REPAINT))
		screen_trashed = TRUE;
}

/*
 * "Toggle" a triple-valued option.
 */
static int
flip_triple(int val, int lc)
{
	if (lc)
		return ((val == OPT_ON) ? OPT_OFF : OPT_ON);
	else
		return ((val == OPT_ONPLUS) ? OPT_OFF : OPT_ONPLUS);
}

/*
 * Determine if an option takes a parameter.
 */
int
opt_has_param(struct loption *o)
{
	if (o == NULL)
		return (0);
	if (o->otype & (BOOL|TRIPLE|NOVAR|NO_TOGGLE))
		return (0);
	return (1);
}

/*
 * Return the prompt to be used for a given option letter.
 * Only string and number valued options have prompts.
 */
char *
opt_prompt(struct loption *o)
{
	if (o == NULL || (o->otype & (STRING|NUMBER)) == 0)
		return ("?");
	return (o->odesc[0]);
}

/*
 * Return whether or not there is a string option pending;
 * that is, if the previous option was a string-valued option letter
 * (like -P) without a following string.
 * In that case, the current option is taken to be the string for
 * the previous option.
 */
int
isoptpending(void)
{
	return (pendopt != NULL);
}

/*
 * Print error message about missing string.
 */
static void
nostring(char *printopt)
{
	PARG parg;
	parg.p_string = printopt;
	error("Value is required after %s", &parg);
}

/*
 * Print error message if a STRING type option is not followed by a string.
 */
void
nopendopt(void)
{
	nostring(opt_desc(pendopt));
}

/*
 * Scan to end of string or to an END_OPTION_STRING character.
 * In the latter case, replace the char with a null char.
 * Return a pointer to the remainder of the string, if any.
 */
static char *
optstring(char *s, char **p_str, char *printopt, char *validchars)
{
	char *p;
	char *out;

	if (*s == '\0') {
		nostring(printopt);
		return (NULL);
	}
	/* Alloc could be more than needed, but not worth trimming. */
	*p_str = ecalloc(strlen(s)+1, sizeof (char));
	out = *p_str;

	for (p = s;  *p != '\0';  p++) {
		if (opt_use_backslash && *p == '\\' && p[1] != '\0') {
			/* Take next char literally. */
			++p;
		} else {
			if (*p == END_OPTION_STRING ||
			    (validchars != NULL &&
			    strchr(validchars, *p) == NULL))
				/* End of option string. */
				break;
		}
		*out++ = *p;
	}
	*out = '\0';
	return (p);
}

/*
 */
static int
num_error(char *printopt, int *errp)
{
	PARG parg;

	if (errp != NULL) {
		*errp = TRUE;
		return (-1);
	}
	if (printopt != NULL) {
		parg.p_string = printopt;
		error("Number is required after %s", &parg);
	}
	return (-1);
}

/*
 * Translate a string into a number.
 * Like atoi(), but takes a pointer to a char *, and updates
 * the char * to point after the translated number.
 */
int
getnum(char **sp, char *printopt, int *errp)
{
	char *s;
	int n;
	int neg;

	s = skipsp(*sp);
	neg = FALSE;
	if (*s == '-') {
		neg = TRUE;
		s++;
	}
	if (*s < '0' || *s > '9')
		return (num_error(printopt, errp));

	n = 0;
	while (*s >= '0' && *s <= '9')
		n = 10 * n + *s++ - '0';
	*sp = s;
	if (errp != NULL)
		*errp = FALSE;
	if (neg)
		n = -n;
	return (n);
}

/*
 * Translate a string into a fraction, represented by the part of a
 * number which would follow a decimal point.
 * The value of the fraction is returned as parts per NUM_FRAC_DENOM.
 * That is, if "n" is returned, the fraction intended is n/NUM_FRAC_DENOM.
 */
long
getfraction(char **sp, char *printopt, int *errp)
{
	char *s;
	long frac = 0;
	int fraclen = 0;

	s = skipsp(*sp);
	if (*s < '0' || *s > '9')
		return (num_error(printopt, errp));

	for (; *s >= '0' && *s <= '9'; s++) {
		frac = (frac * 10) + (*s - '0');
		fraclen++;
	}
	if (fraclen > NUM_LOG_FRAC_DENOM)
		while (fraclen-- > NUM_LOG_FRAC_DENOM)
			frac /= 10;
	else
		while (fraclen++ < NUM_LOG_FRAC_DENOM)
			frac *= 10;
	*sp = s;
	if (errp != NULL)
		*errp = FALSE;
	return (frac);
}


/*
 * Get the value of the -e flag.
 */
int
get_quit_at_eof(void)
{
	return (quit_at_eof);
}
