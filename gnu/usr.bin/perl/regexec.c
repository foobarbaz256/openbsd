/*    regexec.c
 */

/*
 * 	One Ring to rule them all, One Ring to find them
 &
 *     [p.v of _The Lord of the Rings_, opening poem]
 *     [p.50 of _The Lord of the Rings_, I/iii: "The Shadow of the Past"]
 *     [p.254 of _The Lord of the Rings_, II/ii: "The Council of Elrond"]
 */

/* This file contains functions for executing a regular expression.  See
 * also regcomp.c which funnily enough, contains functions for compiling
 * a regular expression.
 *
 * This file is also copied at build time to ext/re/re_exec.c, where
 * it's built with -DPERL_EXT_RE_BUILD -DPERL_EXT_RE_DEBUG -DPERL_EXT.
 * This causes the main functions to be compiled under new names and with
 * debugging support added, which makes "use re 'debug'" work.
 */

/* NOTE: this is derived from Henry Spencer's regexp code, and should not
 * confused with the original package (see point 3 below).  Thanks, Henry!
 */

/* Additional note: this code is very heavily munged from Henry's version
 * in places.  In some spots I've traded clarity for efficiency, so don't
 * blame Henry for some of the lack of readability.
 */

/* The names of the functions have been changed from regcomp and
 * regexec to  pregcomp and pregexec in order to avoid conflicts
 * with the POSIX routines of the same names.
*/

#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

/*
 * pregcomp and pregexec -- regsub and regerror are not used in perl
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 ****    Alterations to Henry's code are...
 ****
 ****    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 ****    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 ****    by Larry Wall and others
 ****
 ****    You may distribute under the terms of either the GNU General Public
 ****    License or the Artistic License, as specified in the README file.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */
#include "EXTERN.h"
#define PERL_IN_REGEXEC_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#define RF_tainted	1		/* tainted information used? */
#define RF_warned	2		/* warned about big count? */

#define RF_utf8		8		/* Pattern contains multibyte chars? */

#define UTF ((PL_reg_flags & RF_utf8) != 0)

#define RS_init		1		/* eval environment created */
#define RS_set		2		/* replsv value is set */

#ifndef STATIC
#define	STATIC	static
#endif

#define REGINCLASS(prog,p,c)  (ANYOF_FLAGS(p) ? reginclass(prog,p,c,0,0) : ANYOF_BITMAP_TEST(p,*(c)))

/*
 * Forwards.
 */

#define CHR_SVLEN(sv) (do_utf8 ? sv_len_utf8(sv) : SvCUR(sv))
#define CHR_DIST(a,b) (PL_reg_match_utf8 ? utf8_distance(a,b) : a - b)

#define HOPc(pos,off) \
	(char *)(PL_reg_match_utf8 \
	    ? reghop3((U8*)pos, off, (U8*)(off >= 0 ? PL_regeol : PL_bostr)) \
	    : (U8*)(pos + off))
#define HOPBACKc(pos, off) \
	(char*)(PL_reg_match_utf8\
	    ? reghopmaybe3((U8*)pos, -off, (U8*)PL_bostr) \
	    : (pos - off >= PL_bostr)		\
		? (U8*)pos - off		\
		: NULL)

#define HOP3(pos,off,lim) (PL_reg_match_utf8 ? reghop3((U8*)(pos), off, (U8*)(lim)) : (U8*)(pos + off))
#define HOP3c(pos,off,lim) ((char*)HOP3(pos,off,lim))

/* these are unrolled below in the CCC_TRY_XXX defined */
#define LOAD_UTF8_CHARCLASS(class,str) STMT_START { \
    if (!CAT2(PL_utf8_,class)) { bool ok; ENTER; save_re_context(); ok=CAT2(is_utf8_,class)((const U8*)str); assert(ok); LEAVE; } } STMT_END

/* Doesn't do an assert to verify that is correct */
#define LOAD_UTF8_CHARCLASS_NO_CHECK(class) STMT_START { \
    if (!CAT2(PL_utf8_,class)) { bool ok; ENTER; save_re_context(); ok=CAT2(is_utf8_,class)((const U8*)" "); LEAVE; } } STMT_END

#define LOAD_UTF8_CHARCLASS_ALNUM() LOAD_UTF8_CHARCLASS(alnum,"a")
#define LOAD_UTF8_CHARCLASS_DIGIT() LOAD_UTF8_CHARCLASS(digit,"0")
#define LOAD_UTF8_CHARCLASS_SPACE() LOAD_UTF8_CHARCLASS(space," ")

#define LOAD_UTF8_CHARCLASS_GCB()  /* Grapheme cluster boundaries */        \
	LOAD_UTF8_CHARCLASS(X_begin, " ");                                  \
	LOAD_UTF8_CHARCLASS(X_non_hangul, "A");                             \
	/* These are utf8 constants, and not utf-ebcdic constants, so the   \
	    * assert should likely and hopefully fail on an EBCDIC machine */ \
	LOAD_UTF8_CHARCLASS(X_extend, "\xcc\x80"); /* U+0300 */             \
									    \
	/* No asserts are done for these, in case called on an early        \
	    * Unicode version in which they map to nothing */               \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_prepend);/* U+0E40 "\xe0\xb9\x80" */ \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_L);	    /* U+1100 "\xe1\x84\x80" */ \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_LV);     /* U+AC00 "\xea\xb0\x80" */ \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_LVT);    /* U+AC01 "\xea\xb0\x81" */ \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_LV_LVT_V);/* U+AC01 "\xea\xb0\x81" */\
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_T);      /* U+11A8 "\xe1\x86\xa8" */ \
	LOAD_UTF8_CHARCLASS_NO_CHECK(X_V)       /* U+1160 "\xe1\x85\xa0" */  

/* 
   We dont use PERL_LEGACY_UNICODE_CHARCLASS_MAPPINGS as the direct test
   so that it is possible to override the option here without having to 
   rebuild the entire core. as we are required to do if we change regcomp.h
   which is where PERL_LEGACY_UNICODE_CHARCLASS_MAPPINGS is defined.
*/
#if PERL_LEGACY_UNICODE_CHARCLASS_MAPPINGS
#define BROKEN_UNICODE_CHARCLASS_MAPPINGS
#endif

#ifdef BROKEN_UNICODE_CHARCLASS_MAPPINGS
#define LOAD_UTF8_CHARCLASS_PERL_WORD()   LOAD_UTF8_CHARCLASS_ALNUM()
#define LOAD_UTF8_CHARCLASS_PERL_SPACE()  LOAD_UTF8_CHARCLASS_SPACE()
#define LOAD_UTF8_CHARCLASS_POSIX_DIGIT() LOAD_UTF8_CHARCLASS_DIGIT()
#define RE_utf8_perl_word   PL_utf8_alnum
#define RE_utf8_perl_space  PL_utf8_space
#define RE_utf8_posix_digit PL_utf8_digit
#define perl_word  alnum
#define perl_space space
#define posix_digit digit
#else
#define LOAD_UTF8_CHARCLASS_PERL_WORD()   LOAD_UTF8_CHARCLASS(perl_word,"a")
#define LOAD_UTF8_CHARCLASS_PERL_SPACE()  LOAD_UTF8_CHARCLASS(perl_space," ")
#define LOAD_UTF8_CHARCLASS_POSIX_DIGIT() LOAD_UTF8_CHARCLASS(posix_digit,"0")
#define RE_utf8_perl_word   PL_utf8_perl_word
#define RE_utf8_perl_space  PL_utf8_perl_space
#define RE_utf8_posix_digit PL_utf8_posix_digit
#endif


#define CCC_TRY_AFF(NAME,NAMEL,CLASS,STR,LCFUNC_utf8,FUNC,LCFUNC)                          \
        case NAMEL:                                                              \
            PL_reg_flags |= RF_tainted;                                                 \
            /* FALL THROUGH */                                                          \
        case NAME:                                                                     \
            if (!nextchr)                                                               \
                sayNO;                                                                  \
            if (do_utf8 && UTF8_IS_CONTINUED(nextchr)) {                                \
                if (!CAT2(PL_utf8_,CLASS)) {                                            \
                    bool ok;                                                            \
                    ENTER;                                                              \
                    save_re_context();                                                  \
                    ok=CAT2(is_utf8_,CLASS)((const U8*)STR);                            \
                    assert(ok);                                                         \
                    LEAVE;                                                              \
                }                                                                       \
                if (!(OP(scan) == NAME                                                  \
                    ? (bool)swash_fetch(CAT2(PL_utf8_,CLASS), (U8*)locinput, do_utf8)   \
                    : LCFUNC_utf8((U8*)locinput)))                                      \
                {                                                                       \
                    sayNO;                                                              \
                }                                                                       \
                locinput += PL_utf8skip[nextchr];                                       \
                nextchr = UCHARAT(locinput);                                            \
                break;                                                                  \
            }                                                                           \
            if (!(OP(scan) == NAME ? FUNC(nextchr) : LCFUNC(nextchr)))                  \
                sayNO;                                                                  \
            nextchr = UCHARAT(++locinput);                                              \
            break

#define CCC_TRY_NEG(NAME,NAMEL,CLASS,STR,LCFUNC_utf8,FUNC,LCFUNC)                        \
        case NAMEL:                                                              \
            PL_reg_flags |= RF_tainted;                                                 \
            /* FALL THROUGH */                                                          \
        case NAME :                                                                     \
            if (!nextchr && locinput >= PL_regeol)                                      \
                sayNO;                                                                  \
            if (do_utf8 && UTF8_IS_CONTINUED(nextchr)) {                                \
                if (!CAT2(PL_utf8_,CLASS)) {                                            \
                    bool ok;                                                            \
                    ENTER;                                                              \
                    save_re_context();                                                  \
                    ok=CAT2(is_utf8_,CLASS)((const U8*)STR);                            \
                    assert(ok);                                                         \
                    LEAVE;                                                              \
                }                                                                       \
                if ((OP(scan) == NAME                                                  \
                    ? (bool)swash_fetch(CAT2(PL_utf8_,CLASS), (U8*)locinput, do_utf8)    \
                    : LCFUNC_utf8((U8*)locinput)))                                      \
                {                                                                       \
                    sayNO;                                                              \
                }                                                                       \
                locinput += PL_utf8skip[nextchr];                                       \
                nextchr = UCHARAT(locinput);                                            \
                break;                                                                  \
            }                                                                           \
            if ((OP(scan) == NAME ? FUNC(nextchr) : LCFUNC(nextchr)))                   \
                sayNO;                                                                  \
            nextchr = UCHARAT(++locinput);                                              \
            break





/* TODO: Combine JUMPABLE and HAS_TEXT to cache OP(rn) */

/* for use after a quantifier and before an EXACT-like node -- japhy */
/* it would be nice to rework regcomp.sym to generate this stuff. sigh */
#define JUMPABLE(rn) (      \
    OP(rn) == OPEN ||       \
    (OP(rn) == CLOSE && (!cur_eval || cur_eval->u.eval.close_paren != ARG(rn))) || \
    OP(rn) == EVAL ||   \
    OP(rn) == SUSPEND || OP(rn) == IFMATCH || \
    OP(rn) == PLUS || OP(rn) == MINMOD || \
    OP(rn) == KEEPS || (PL_regkind[OP(rn)] == VERB) || \
    (PL_regkind[OP(rn)] == CURLY && ARG1(rn) > 0) \
)
#define IS_EXACT(rn) (PL_regkind[OP(rn)] == EXACT)

#define HAS_TEXT(rn) ( IS_EXACT(rn) || PL_regkind[OP(rn)] == REF )

#if 0 
/* Currently these are only used when PL_regkind[OP(rn)] == EXACT so
   we don't need this definition. */
#define IS_TEXT(rn)   ( OP(rn)==EXACT   || OP(rn)==REF   || OP(rn)==NREF   )
#define IS_TEXTF(rn)  ( OP(rn)==EXACTF  || OP(rn)==REFF  || OP(rn)==NREFF  )
#define IS_TEXTFL(rn) ( OP(rn)==EXACTFL || OP(rn)==REFFL || OP(rn)==NREFFL )

#else
/* ... so we use this as its faster. */
#define IS_TEXT(rn)   ( OP(rn)==EXACT   )
#define IS_TEXTF(rn)  ( OP(rn)==EXACTF  )
#define IS_TEXTFL(rn) ( OP(rn)==EXACTFL )

#endif

/*
  Search for mandatory following text node; for lookahead, the text must
  follow but for lookbehind (rn->flags != 0) we skip to the next step.
*/
#define FIND_NEXT_IMPT(rn) STMT_START { \
    while (JUMPABLE(rn)) { \
	const OPCODE type = OP(rn); \
	if (type == SUSPEND || PL_regkind[type] == CURLY) \
	    rn = NEXTOPER(NEXTOPER(rn)); \
	else if (type == PLUS) \
	    rn = NEXTOPER(rn); \
	else if (type == IFMATCH) \
	    rn = (rn->flags == 0) ? NEXTOPER(NEXTOPER(rn)) : rn + ARG(rn); \
	else rn += NEXT_OFF(rn); \
    } \
} STMT_END 


static void restore_pos(pTHX_ void *arg);

STATIC CHECKPOINT
S_regcppush(pTHX_ I32 parenfloor)
{
    dVAR;
    const int retval = PL_savestack_ix;
#define REGCP_PAREN_ELEMS 4
    const int paren_elems_to_push = (PL_regsize - parenfloor) * REGCP_PAREN_ELEMS;
    int p;
    GET_RE_DEBUG_FLAGS_DECL;

    if (paren_elems_to_push < 0)
	Perl_croak(aTHX_ "panic: paren_elems_to_push < 0");

#define REGCP_OTHER_ELEMS 7
    SSGROW(paren_elems_to_push + REGCP_OTHER_ELEMS);
    
    for (p = PL_regsize; p > parenfloor; p--) {
/* REGCP_PARENS_ELEMS are pushed per pairs of parentheses. */
	SSPUSHINT(PL_regoffs[p].end);
	SSPUSHINT(PL_regoffs[p].start);
	SSPUSHPTR(PL_reg_start_tmp[p]);
	SSPUSHINT(p);
	DEBUG_BUFFERS_r(PerlIO_printf(Perl_debug_log,
	  "     saving \\%"UVuf" %"IVdf"(%"IVdf")..%"IVdf"\n",
		      (UV)p, (IV)PL_regoffs[p].start,
		      (IV)(PL_reg_start_tmp[p] - PL_bostr),
		      (IV)PL_regoffs[p].end
	));
    }
/* REGCP_OTHER_ELEMS are pushed in any case, parentheses or no. */
    SSPUSHPTR(PL_regoffs);
    SSPUSHINT(PL_regsize);
    SSPUSHINT(*PL_reglastparen);
    SSPUSHINT(*PL_reglastcloseparen);
    SSPUSHPTR(PL_reginput);
#define REGCP_FRAME_ELEMS 2
/* REGCP_FRAME_ELEMS are part of the REGCP_OTHER_ELEMS and
 * are needed for the regexp context stack bookkeeping. */
    SSPUSHINT(paren_elems_to_push + REGCP_OTHER_ELEMS - REGCP_FRAME_ELEMS);
    SSPUSHINT(SAVEt_REGCONTEXT); /* Magic cookie. */

    return retval;
}

/* These are needed since we do not localize EVAL nodes: */
#define REGCP_SET(cp)                                           \
    DEBUG_STATE_r(                                              \
            PerlIO_printf(Perl_debug_log,		        \
	        "  Setting an EVAL scope, savestack=%"IVdf"\n",	\
	        (IV)PL_savestack_ix));                          \
    cp = PL_savestack_ix

#define REGCP_UNWIND(cp)                                        \
    DEBUG_STATE_r(                                              \
        if (cp != PL_savestack_ix) 		                \
    	    PerlIO_printf(Perl_debug_log,		        \
		"  Clearing an EVAL scope, savestack=%"IVdf"..%"IVdf"\n", \
	        (IV)(cp), (IV)PL_savestack_ix));                \
    regcpblow(cp)

STATIC char *
S_regcppop(pTHX_ const regexp *rex)
{
    dVAR;
    U32 i;
    char *input;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGCPPOP;

    /* Pop REGCP_OTHER_ELEMS before the parentheses loop starts. */
    i = SSPOPINT;
    assert(i == SAVEt_REGCONTEXT); /* Check that the magic cookie is there. */
    i = SSPOPINT; /* Parentheses elements to pop. */
    input = (char *) SSPOPPTR;
    *PL_reglastcloseparen = SSPOPINT;
    *PL_reglastparen = SSPOPINT;
    PL_regsize = SSPOPINT;
    PL_regoffs=(regexp_paren_pair *) SSPOPPTR;

    
    /* Now restore the parentheses context. */
    for (i -= (REGCP_OTHER_ELEMS - REGCP_FRAME_ELEMS);
	 i > 0; i -= REGCP_PAREN_ELEMS) {
	I32 tmps;
	U32 paren = (U32)SSPOPINT;
	PL_reg_start_tmp[paren] = (char *) SSPOPPTR;
	PL_regoffs[paren].start = SSPOPINT;
	tmps = SSPOPINT;
	if (paren <= *PL_reglastparen)
	    PL_regoffs[paren].end = tmps;
	DEBUG_BUFFERS_r(
	    PerlIO_printf(Perl_debug_log,
			  "     restoring \\%"UVuf" to %"IVdf"(%"IVdf")..%"IVdf"%s\n",
			  (UV)paren, (IV)PL_regoffs[paren].start,
			  (IV)(PL_reg_start_tmp[paren] - PL_bostr),
			  (IV)PL_regoffs[paren].end,
			  (paren > *PL_reglastparen ? "(no)" : ""));
	);
    }
    DEBUG_BUFFERS_r(
	if (*PL_reglastparen + 1 <= rex->nparens) {
	    PerlIO_printf(Perl_debug_log,
			  "     restoring \\%"IVdf"..\\%"IVdf" to undef\n",
			  (IV)(*PL_reglastparen + 1), (IV)rex->nparens);
	}
    );
#if 1
    /* It would seem that the similar code in regtry()
     * already takes care of this, and in fact it is in
     * a better location to since this code can #if 0-ed out
     * but the code in regtry() is needed or otherwise tests
     * requiring null fields (pat.t#187 and split.t#{13,14}
     * (as of patchlevel 7877)  will fail.  Then again,
     * this code seems to be necessary or otherwise
     * this erroneously leaves $1 defined: "1" =~ /^(?:(\d)x)?\d$/
     * --jhi updated by dapm */
    for (i = *PL_reglastparen + 1; i <= rex->nparens; i++) {
	if (i > PL_regsize)
	    PL_regoffs[i].start = -1;
	PL_regoffs[i].end = -1;
    }
#endif
    return input;
}

#define regcpblow(cp) LEAVE_SCOPE(cp)	/* Ignores regcppush()ed data. */

/*
 * pregexec and friends
 */

#ifndef PERL_IN_XSUB_RE
/*
 - pregexec - match a regexp against a string
 */
I32
Perl_pregexec(pTHX_ REGEXP * const prog, char* stringarg, register char *strend,
	 char *strbeg, I32 minend, SV *screamer, U32 nosave)
/* strend: pointer to null at end of string */
/* strbeg: real beginning of string */
/* minend: end of match must be >=minend after stringarg. */
/* nosave: For optimizations. */
{
    PERL_ARGS_ASSERT_PREGEXEC;

    return
	regexec_flags(prog, stringarg, strend, strbeg, minend, screamer, NULL,
		      nosave ? 0 : REXEC_COPY_STR);
}
#endif

/*
 * Need to implement the following flags for reg_anch:
 *
 * USE_INTUIT_NOML		- Useful to call re_intuit_start() first
 * USE_INTUIT_ML
 * INTUIT_AUTORITATIVE_NOML	- Can trust a positive answer
 * INTUIT_AUTORITATIVE_ML
 * INTUIT_ONCE_NOML		- Intuit can match in one location only.
 * INTUIT_ONCE_ML
 *
 * Another flag for this function: SECOND_TIME (so that float substrs
 * with giant delta may be not rechecked).
 */

/* Assumptions: if ANCH_GPOS, then strpos is anchored. XXXX Check GPOS logic */

/* If SCREAM, then SvPVX_const(sv) should be compatible with strpos and strend.
   Otherwise, only SvCUR(sv) is used to get strbeg. */

/* XXXX We assume that strpos is strbeg unless sv. */

/* XXXX Some places assume that there is a fixed substring.
	An update may be needed if optimizer marks as "INTUITable"
	RExen without fixed substrings.  Similarly, it is assumed that
	lengths of all the strings are no more than minlen, thus they
	cannot come from lookahead.
	(Or minlen should take into account lookahead.) 
  NOTE: Some of this comment is not correct. minlen does now take account
  of lookahead/behind. Further research is required. -- demerphq

*/

/* A failure to find a constant substring means that there is no need to make
   an expensive call to REx engine, thus we celebrate a failure.  Similarly,
   finding a substring too deep into the string means that less calls to
   regtry() should be needed.

   REx compiler's optimizer found 4 possible hints:
	a) Anchored substring;
	b) Fixed substring;
	c) Whether we are anchored (beginning-of-line or \G);
	d) First node (of those at offset 0) which may distingush positions;
   We use a)b)d) and multiline-part of c), and try to find a position in the
   string which does not contradict any of them.
 */

/* Most of decisions we do here should have been done at compile time.
   The nodes of the REx which we used for the search should have been
   deleted from the finite automaton. */

char *
Perl_re_intuit_start(pTHX_ REGEXP * const rx, SV *sv, char *strpos,
		     char *strend, const U32 flags, re_scream_pos_data *data)
{
    dVAR;
    struct regexp *const prog = (struct regexp *)SvANY(rx);
    register I32 start_shift = 0;
    /* Should be nonnegative! */
    register I32 end_shift   = 0;
    register char *s;
    register SV *check;
    char *strbeg;
    char *t;
    const bool do_utf8 = (sv && SvUTF8(sv)) ? 1 : 0; /* if no sv we have to assume bytes */
    I32 ml_anch;
    register char *other_last = NULL;	/* other substr checked before this */
    char *check_at = NULL;		/* check substr found at this pos */
    const I32 multiline = prog->extflags & RXf_PMf_MULTILINE;
    RXi_GET_DECL(prog,progi);
#ifdef DEBUGGING
    const char * const i_strpos = strpos;
#endif
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_RE_INTUIT_START;

    RX_MATCH_UTF8_set(rx,do_utf8);

    if (RX_UTF8(rx)) {
	PL_reg_flags |= RF_utf8;
    }
    DEBUG_EXECUTE_r( 
        debug_start_match(rx, do_utf8, strpos, strend, 
            sv ? "Guessing start of match in sv for"
               : "Guessing start of match in string for");
	      );

    /* CHR_DIST() would be more correct here but it makes things slow. */
    if (prog->minlen > strend - strpos) {
	DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
			      "String too short... [re_intuit_start]\n"));
	goto fail;
    }
                
    strbeg = (sv && SvPOK(sv)) ? strend - SvCUR(sv) : strpos;
    PL_regeol = strend;
    if (do_utf8) {
	if (!prog->check_utf8 && prog->check_substr)
	    to_utf8_substr(prog);
	check = prog->check_utf8;
    } else {
	if (!prog->check_substr && prog->check_utf8)
	    to_byte_substr(prog);
	check = prog->check_substr;
    }
    if (check == &PL_sv_undef) {
	DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		"Non-utf8 string cannot match utf8 check string\n"));
	goto fail;
    }
    if (prog->extflags & RXf_ANCH) {	/* Match at beg-of-str or after \n */
	ml_anch = !( (prog->extflags & RXf_ANCH_SINGLE)
		     || ( (prog->extflags & RXf_ANCH_BOL)
			  && !multiline ) );	/* Check after \n? */

	if (!ml_anch) {
	  if ( !(prog->extflags & RXf_ANCH_GPOS) /* Checked by the caller */
		&& !(prog->intflags & PREGf_IMPLICIT) /* not a real BOL */
	       /* SvCUR is not set on references: SvRV and SvPVX_const overlap */
	       && sv && !SvROK(sv)
	       && (strpos != strbeg)) {
	      DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Not at start...\n"));
	      goto fail;
	  }
	  if (prog->check_offset_min == prog->check_offset_max &&
	      !(prog->extflags & RXf_CANY_SEEN)) {
	    /* Substring at constant offset from beg-of-str... */
	    I32 slen;

	    s = HOP3c(strpos, prog->check_offset_min, strend);
	    
	    if (SvTAIL(check)) {
		slen = SvCUR(check);	/* >= 1 */

		if ( strend - s > slen || strend - s < slen - 1
		     || (strend - s == slen && strend[-1] != '\n')) {
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "String too long...\n"));
		    goto fail_finish;
		}
		/* Now should match s[0..slen-2] */
		slen--;
		if (slen && (*SvPVX_const(check) != *s
			     || (slen > 1
				 && memNE(SvPVX_const(check), s, slen)))) {
		  report_neq:
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "String not equal...\n"));
		    goto fail_finish;
		}
	    }
	    else if (*SvPVX_const(check) != *s
		     || ((slen = SvCUR(check)) > 1
			 && memNE(SvPVX_const(check), s, slen)))
		goto report_neq;
	    check_at = s;
	    goto success_at_start;
	  }
	}
	/* Match is anchored, but substr is not anchored wrt beg-of-str. */
	s = strpos;
	start_shift = prog->check_offset_min; /* okay to underestimate on CC */
	end_shift = prog->check_end_shift;
	
	if (!ml_anch) {
	    const I32 end = prog->check_offset_max + CHR_SVLEN(check)
					 - (SvTAIL(check) != 0);
	    const I32 eshift = CHR_DIST((U8*)strend, (U8*)s) - end;

	    if (end_shift < eshift)
		end_shift = eshift;
	}
    }
    else {				/* Can match at random position */
	ml_anch = 0;
	s = strpos;
	start_shift = prog->check_offset_min;  /* okay to underestimate on CC */
	end_shift = prog->check_end_shift;
	
	/* end shift should be non negative here */
    }

#ifdef QDEBUGGING	/* 7/99: reports of failure (with the older version) */
    if (end_shift < 0)
	Perl_croak(aTHX_ "panic: end_shift: %"IVdf" pattern:\n%s\n ",
		   (IV)end_shift, RX_PRECOMP(prog));
#endif

  restart:
    /* Find a possible match in the region s..strend by looking for
       the "check" substring in the region corrected by start/end_shift. */
    
    {
        I32 srch_start_shift = start_shift;
        I32 srch_end_shift = end_shift;
        if (srch_start_shift < 0 && strbeg - s > srch_start_shift) {
	    srch_end_shift -= ((strbeg - s) - srch_start_shift); 
	    srch_start_shift = strbeg - s;
	}
    DEBUG_OPTIMISE_MORE_r({
        PerlIO_printf(Perl_debug_log, "Check offset min: %"IVdf" Start shift: %"IVdf" End shift %"IVdf" Real End Shift: %"IVdf"\n",
            (IV)prog->check_offset_min,
            (IV)srch_start_shift,
            (IV)srch_end_shift, 
            (IV)prog->check_end_shift);
    });       
        
    if (flags & REXEC_SCREAM) {
	I32 p = -1;			/* Internal iterator of scream. */
	I32 * const pp = data ? data->scream_pos : &p;

	if (PL_screamfirst[BmRARE(check)] >= 0
	    || ( BmRARE(check) == '\n'
		 && (BmPREVIOUS(check) == SvCUR(check) - 1)
		 && SvTAIL(check) ))
	    s = screaminstr(sv, check,
			    srch_start_shift + (s - strbeg), srch_end_shift, pp, 0);
	else
	    goto fail_finish;
	/* we may be pointing at the wrong string */
	if (s && RXp_MATCH_COPIED(prog))
	    s = strbeg + (s - SvPVX_const(sv));
	if (data)
	    *data->scream_olds = s;
    }
    else {
        U8* start_point;
        U8* end_point;
        if (prog->extflags & RXf_CANY_SEEN) {
            start_point= (U8*)(s + srch_start_shift);
            end_point= (U8*)(strend - srch_end_shift);
        } else {
	    start_point= HOP3(s, srch_start_shift, srch_start_shift < 0 ? strbeg : strend);
            end_point= HOP3(strend, -srch_end_shift, strbeg);
	}
	DEBUG_OPTIMISE_MORE_r({
            PerlIO_printf(Perl_debug_log, "fbm_instr len=%d str=<%.*s>\n", 
                (int)(end_point - start_point),
                (int)(end_point - start_point) > 20 ? 20 : (int)(end_point - start_point), 
                start_point);
        });

	s = fbm_instr( start_point, end_point,
		      check, multiline ? FBMrf_MULTILINE : 0);
    }
    }
    /* Update the count-of-usability, remove useless subpatterns,
	unshift s.  */

    DEBUG_EXECUTE_r({
        RE_PV_QUOTED_DECL(quoted, do_utf8, PERL_DEBUG_PAD_ZERO(0), 
            SvPVX_const(check), RE_SV_DUMPLEN(check), 30);
        PerlIO_printf(Perl_debug_log, "%s %s substr %s%s%s",
			  (s ? "Found" : "Did not find"),
	    (check == (do_utf8 ? prog->anchored_utf8 : prog->anchored_substr) 
	        ? "anchored" : "floating"),
	    quoted,
	    RE_SV_TAIL(check),
	    (s ? " at offset " : "...\n") ); 
    });

    if (!s)
	goto fail_finish;
    /* Finish the diagnostic message */
    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%ld...\n", (long)(s - i_strpos)) );

    /* XXX dmq: first branch is for positive lookbehind...
       Our check string is offset from the beginning of the pattern.
       So we need to do any stclass tests offset forward from that 
       point. I think. :-(
     */
    
        
    
    check_at=s;
     

    /* Got a candidate.  Check MBOL anchoring, and the *other* substr.
       Start with the other substr.
       XXXX no SCREAM optimization yet - and a very coarse implementation
       XXXX /ttx+/ results in anchored="ttx", floating="x".  floating will
		*always* match.  Probably should be marked during compile...
       Probably it is right to do no SCREAM here...
     */

    if (do_utf8 ? (prog->float_utf8 && prog->anchored_utf8) 
                : (prog->float_substr && prog->anchored_substr)) 
    {
	/* Take into account the "other" substring. */
	/* XXXX May be hopelessly wrong for UTF... */
	if (!other_last)
	    other_last = strpos;
	if (check == (do_utf8 ? prog->float_utf8 : prog->float_substr)) {
	  do_other_anchored:
	    {
		char * const last = HOP3c(s, -start_shift, strbeg);
		char *last1, *last2;
		char * const saved_s = s;
		SV* must;

		t = s - prog->check_offset_max;
		if (s - strpos > prog->check_offset_max  /* signed-corrected t > strpos */
		    && (!do_utf8
			|| ((t = (char*)reghopmaybe3((U8*)s, -(prog->check_offset_max), (U8*)strpos))
			    && t > strpos)))
		    NOOP;
		else
		    t = strpos;
		t = HOP3c(t, prog->anchored_offset, strend);
		if (t < other_last)	/* These positions already checked */
		    t = other_last;
		last2 = last1 = HOP3c(strend, -prog->minlen, strbeg);
		if (last < last1)
		    last1 = last;
                /* XXXX It is not documented what units *_offsets are in.  
                   We assume bytes, but this is clearly wrong. 
                   Meaning this code needs to be carefully reviewed for errors.
                   dmq.
                  */
 
		/* On end-of-str: see comment below. */
		must = do_utf8 ? prog->anchored_utf8 : prog->anchored_substr;
		if (must == &PL_sv_undef) {
		    s = (char*)NULL;
		    DEBUG_r(must = prog->anchored_utf8);	/* for debug */
		}
		else
		    s = fbm_instr(
			(unsigned char*)t,
			HOP3(HOP3(last1, prog->anchored_offset, strend)
				+ SvCUR(must), -(SvTAIL(must)!=0), strbeg),
			must,
			multiline ? FBMrf_MULTILINE : 0
		    );
                DEBUG_EXECUTE_r({
                    RE_PV_QUOTED_DECL(quoted, do_utf8, PERL_DEBUG_PAD_ZERO(0), 
                        SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
                    PerlIO_printf(Perl_debug_log, "%s anchored substr %s%s",
			(s ? "Found" : "Contradicts"),
                        quoted, RE_SV_TAIL(must));
                });		    
		
			    
		if (!s) {
		    if (last1 >= last2) {
			DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
						", giving up...\n"));
			goto fail_finish;
		    }
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
			", trying floating at offset %ld...\n",
			(long)(HOP3c(saved_s, 1, strend) - i_strpos)));
		    other_last = HOP3c(last1, prog->anchored_offset+1, strend);
		    s = HOP3c(last, 1, strend);
		    goto restart;
		}
		else {
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, " at offset %ld...\n",
			  (long)(s - i_strpos)));
		    t = HOP3c(s, -prog->anchored_offset, strbeg);
		    other_last = HOP3c(s, 1, strend);
		    s = saved_s;
		    if (t == strpos)
			goto try_at_start;
		    goto try_at_offset;
		}
	    }
	}
	else {		/* Take into account the floating substring. */
	    char *last, *last1;
	    char * const saved_s = s;
	    SV* must;

	    t = HOP3c(s, -start_shift, strbeg);
	    last1 = last =
		HOP3c(strend, -prog->minlen + prog->float_min_offset, strbeg);
	    if (CHR_DIST((U8*)last, (U8*)t) > prog->float_max_offset)
		last = HOP3c(t, prog->float_max_offset, strend);
	    s = HOP3c(t, prog->float_min_offset, strend);
	    if (s < other_last)
		s = other_last;
 /* XXXX It is not documented what units *_offsets are in.  Assume bytes.  */
	    must = do_utf8 ? prog->float_utf8 : prog->float_substr;
	    /* fbm_instr() takes into account exact value of end-of-str
	       if the check is SvTAIL(ed).  Since false positives are OK,
	       and end-of-str is not later than strend we are OK. */
	    if (must == &PL_sv_undef) {
		s = (char*)NULL;
		DEBUG_r(must = prog->float_utf8);	/* for debug message */
	    }
	    else
		s = fbm_instr((unsigned char*)s,
			      (unsigned char*)last + SvCUR(must)
				  - (SvTAIL(must)!=0),
			      must, multiline ? FBMrf_MULTILINE : 0);
	    DEBUG_EXECUTE_r({
	        RE_PV_QUOTED_DECL(quoted, do_utf8, PERL_DEBUG_PAD_ZERO(0), 
	            SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
	        PerlIO_printf(Perl_debug_log, "%s floating substr %s%s",
		    (s ? "Found" : "Contradicts"),
		    quoted, RE_SV_TAIL(must));
            });
	    if (!s) {
		if (last1 == last) {
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
					    ", giving up...\n"));
		    goto fail_finish;
		}
		DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		    ", trying anchored starting at offset %ld...\n",
		    (long)(saved_s + 1 - i_strpos)));
		other_last = last;
		s = HOP3c(t, 1, strend);
		goto restart;
	    }
	    else {
		DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, " at offset %ld...\n",
		      (long)(s - i_strpos)));
		other_last = s; /* Fix this later. --Hugo */
		s = saved_s;
		if (t == strpos)
		    goto try_at_start;
		goto try_at_offset;
	    }
	}
    }

    
    t= (char*)HOP3( s, -prog->check_offset_max, (prog->check_offset_max<0) ? strend : strpos);
        
    DEBUG_OPTIMISE_MORE_r(
        PerlIO_printf(Perl_debug_log, 
            "Check offset min:%"IVdf" max:%"IVdf" S:%"IVdf" t:%"IVdf" D:%"IVdf" end:%"IVdf"\n",
            (IV)prog->check_offset_min,
            (IV)prog->check_offset_max,
            (IV)(s-strpos),
            (IV)(t-strpos),
            (IV)(t-s),
            (IV)(strend-strpos)
        )
    );

    if (s - strpos > prog->check_offset_max  /* signed-corrected t > strpos */
        && (!do_utf8
	    || ((t = (char*)reghopmaybe3((U8*)s, -prog->check_offset_max, (U8*) ((prog->check_offset_max<0) ? strend : strpos)))
		 && t > strpos))) 
    {
	/* Fixed substring is found far enough so that the match
	   cannot start at strpos. */
      try_at_offset:
	if (ml_anch && t[-1] != '\n') {
	    /* Eventually fbm_*() should handle this, but often
	       anchored_offset is not 0, so this check will not be wasted. */
	    /* XXXX In the code below we prefer to look for "^" even in
	       presence of anchored substrings.  And we search even
	       beyond the found float position.  These pessimizations
	       are historical artefacts only.  */
	  find_anchor:
	    while (t < strend - prog->minlen) {
		if (*t == '\n') {
		    if (t < check_at - prog->check_offset_min) {
			if (do_utf8 ? prog->anchored_utf8 : prog->anchored_substr) {
			    /* Since we moved from the found position,
			       we definitely contradict the found anchored
			       substr.  Due to the above check we do not
			       contradict "check" substr.
			       Thus we can arrive here only if check substr
			       is float.  Redo checking for "other"=="fixed".
			     */
			    strpos = t + 1;			
			    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Found /%s^%s/m at offset %ld, rescanning for anchored from offset %ld...\n",
				PL_colors[0], PL_colors[1], (long)(strpos - i_strpos), (long)(strpos - i_strpos + prog->anchored_offset)));
			    goto do_other_anchored;
			}
			/* We don't contradict the found floating substring. */
			/* XXXX Why not check for STCLASS? */
			s = t + 1;
			DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Found /%s^%s/m at offset %ld...\n",
			    PL_colors[0], PL_colors[1], (long)(s - i_strpos)));
			goto set_useful;
		    }
		    /* Position contradicts check-string */
		    /* XXXX probably better to look for check-string
		       than for "\n", so one should lower the limit for t? */
		    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Found /%s^%s/m, restarting lookup for check-string at offset %ld...\n",
			PL_colors[0], PL_colors[1], (long)(t + 1 - i_strpos)));
		    other_last = strpos = s = t + 1;
		    goto restart;
		}
		t++;
	    }
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Did not find /%s^%s/m...\n",
			PL_colors[0], PL_colors[1]));
	    goto fail_finish;
	}
	else {
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Starting position does not contradict /%s^%s/m...\n",
			PL_colors[0], PL_colors[1]));
	}
	s = t;
      set_useful:
	++BmUSEFUL(do_utf8 ? prog->check_utf8 : prog->check_substr);	/* hooray/5 */
    }
    else {
	/* The found string does not prohibit matching at strpos,
	   - no optimization of calling REx engine can be performed,
	   unless it was an MBOL and we are not after MBOL,
	   or a future STCLASS check will fail this. */
      try_at_start:
	/* Even in this situation we may use MBOL flag if strpos is offset
	   wrt the start of the string. */
	if (ml_anch && sv && !SvROK(sv)	/* See prev comment on SvROK */
	    && (strpos != strbeg) && strpos[-1] != '\n'
	    /* May be due to an implicit anchor of m{.*foo}  */
	    && !(prog->intflags & PREGf_IMPLICIT))
	{
	    t = strpos;
	    goto find_anchor;
	}
	DEBUG_EXECUTE_r( if (ml_anch)
	    PerlIO_printf(Perl_debug_log, "Position at offset %ld does not contradict /%s^%s/m...\n",
			  (long)(strpos - i_strpos), PL_colors[0], PL_colors[1]);
	);
      success_at_start:
	if (!(prog->intflags & PREGf_NAUGHTY)	/* XXXX If strpos moved? */
	    && (do_utf8 ? (
		prog->check_utf8		/* Could be deleted already */
		&& --BmUSEFUL(prog->check_utf8) < 0
		&& (prog->check_utf8 == prog->float_utf8)
	    ) : (
		prog->check_substr		/* Could be deleted already */
		&& --BmUSEFUL(prog->check_substr) < 0
		&& (prog->check_substr == prog->float_substr)
	    )))
	{
	    /* If flags & SOMETHING - do not do it many times on the same match */
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "... Disabling check substring...\n"));
	    /* XXX Does the destruction order has to change with do_utf8? */
	    SvREFCNT_dec(do_utf8 ? prog->check_utf8 : prog->check_substr);
	    SvREFCNT_dec(do_utf8 ? prog->check_substr : prog->check_utf8);
	    prog->check_substr = prog->check_utf8 = NULL;	/* disable */
	    prog->float_substr = prog->float_utf8 = NULL;	/* clear */
	    check = NULL;			/* abort */
	    s = strpos;
	    /* XXXX This is a remnant of the old implementation.  It
	            looks wasteful, since now INTUIT can use many
	            other heuristics. */
	    prog->extflags &= ~RXf_USE_INTUIT;
	}
	else
	    s = strpos;
    }

    /* Last resort... */
    /* XXXX BmUSEFUL already changed, maybe multiple change is meaningful... */
    /* trie stclasses are too expensive to use here, we are better off to
       leave it to regmatch itself */
    if (progi->regstclass && PL_regkind[OP(progi->regstclass)]!=TRIE) {
	/* minlen == 0 is possible if regstclass is \b or \B,
	   and the fixed substr is ''$.
	   Since minlen is already taken into account, s+1 is before strend;
	   accidentally, minlen >= 1 guaranties no false positives at s + 1
	   even for \b or \B.  But (minlen? 1 : 0) below assumes that
	   regstclass does not come from lookahead...  */
	/* If regstclass takes bytelength more than 1: If charlength==1, OK.
	   This leaves EXACTF only, which is dealt with in find_byclass().  */
        const U8* const str = (U8*)STRING(progi->regstclass);
        const int cl_l = (PL_regkind[OP(progi->regstclass)] == EXACT
		    ? CHR_DIST(str+STR_LEN(progi->regstclass), str)
		    : 1);
	char * endpos;
	if (prog->anchored_substr || prog->anchored_utf8 || ml_anch)
            endpos= HOP3c(s, (prog->minlen ? cl_l : 0), strend);
        else if (prog->float_substr || prog->float_utf8)
	    endpos= HOP3c(HOP3c(check_at, -start_shift, strbeg), cl_l, strend);
        else 
            endpos= strend;
		    
        DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "start_shift: %"IVdf" check_at: %"IVdf" s: %"IVdf" endpos: %"IVdf"\n",
				      (IV)start_shift, (IV)(check_at - strbeg), (IV)(s - strbeg), (IV)(endpos - strbeg)));
	
	t = s;
        s = find_byclass(prog, progi->regstclass, s, endpos, NULL);
	if (!s) {
#ifdef DEBUGGING
	    const char *what = NULL;
#endif
	    if (endpos == strend) {
		DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
				"Could not match STCLASS...\n") );
		goto fail;
	    }
	    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
				   "This position contradicts STCLASS...\n") );
	    if ((prog->extflags & RXf_ANCH) && !ml_anch)
		goto fail;
	    /* Contradict one of substrings */
	    if (prog->anchored_substr || prog->anchored_utf8) {
		if ((do_utf8 ? prog->anchored_utf8 : prog->anchored_substr) == check) {
		    DEBUG_EXECUTE_r( what = "anchored" );
		  hop_and_restart:
		    s = HOP3c(t, 1, strend);
		    if (s + start_shift + end_shift > strend) {
			/* XXXX Should be taken into account earlier? */
			DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
					       "Could not match STCLASS...\n") );
			goto fail;
		    }
		    if (!check)
			goto giveup;
		    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
				"Looking for %s substr starting at offset %ld...\n",
				 what, (long)(s + start_shift - i_strpos)) );
		    goto restart;
		}
		/* Have both, check_string is floating */
		if (t + start_shift >= check_at) /* Contradicts floating=check */
		    goto retry_floating_check;
		/* Recheck anchored substring, but not floating... */
		s = check_at;
		if (!check)
		    goto giveup;
		DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
			  "Looking for anchored substr starting at offset %ld...\n",
			  (long)(other_last - i_strpos)) );
		goto do_other_anchored;
	    }
	    /* Another way we could have checked stclass at the
               current position only: */
	    if (ml_anch) {
		s = t = t + 1;
		if (!check)
		    goto giveup;
		DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
			  "Looking for /%s^%s/m starting at offset %ld...\n",
			  PL_colors[0], PL_colors[1], (long)(t - i_strpos)) );
		goto try_at_offset;
	    }
	    if (!(do_utf8 ? prog->float_utf8 : prog->float_substr))	/* Could have been deleted */
		goto fail;
	    /* Check is floating subtring. */
	  retry_floating_check:
	    t = check_at - start_shift;
	    DEBUG_EXECUTE_r( what = "floating" );
	    goto hop_and_restart;
	}
	if (t != s) {
            DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
			"By STCLASS: moving %ld --> %ld\n",
                                  (long)(t - i_strpos), (long)(s - i_strpos))
                   );
        }
        else {
            DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
                                  "Does not contradict STCLASS...\n"); 
                   );
        }
    }
  giveup:
    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%s%s:%s match at offset %ld\n",
			  PL_colors[4], (check ? "Guessed" : "Giving up"),
			  PL_colors[5], (long)(s - i_strpos)) );
    return s;

  fail_finish:				/* Substring not found */
    if (prog->check_substr || prog->check_utf8)		/* could be removed already */
	BmUSEFUL(do_utf8 ? prog->check_utf8 : prog->check_substr) += 5; /* hooray */
  fail:
    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%sMatch rejected by optimizer%s\n",
			  PL_colors[4], PL_colors[5]));
    return NULL;
}

#define DECL_TRIE_TYPE(scan) \
    const enum { trie_plain, trie_utf8, trie_utf8_fold, trie_latin_utf8_fold } \
		    trie_type = (scan->flags != EXACT) \
		              ? (do_utf8 ? trie_utf8_fold : (UTF ? trie_latin_utf8_fold : trie_plain)) \
                              : (do_utf8 ? trie_utf8 : trie_plain)

#define REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc, uscan, len,  \
uvc, charid, foldlen, foldbuf, uniflags) STMT_START {                       \
    switch (trie_type) {                                                    \
    case trie_utf8_fold:                                                    \
	if ( foldlen>0 ) {                                                  \
	    uvc = utf8n_to_uvuni( uscan, UTF8_MAXLEN, &len, uniflags ); \
	    foldlen -= len;                                                 \
	    uscan += len;                                                   \
	    len=0;                                                          \
	} else {                                                            \
	    uvc = utf8n_to_uvuni( (U8*)uc, UTF8_MAXLEN, &len, uniflags ); \
	    uvc = to_uni_fold( uvc, foldbuf, &foldlen );                    \
	    foldlen -= UNISKIP( uvc );                                      \
	    uscan = foldbuf + UNISKIP( uvc );                               \
	}                                                                   \
	break;                                                              \
    case trie_latin_utf8_fold:                                              \
	if ( foldlen>0 ) {                                                  \
	    uvc = utf8n_to_uvuni( uscan, UTF8_MAXLEN, &len, uniflags );     \
	    foldlen -= len;                                                 \
	    uscan += len;                                                   \
	    len=0;                                                          \
	} else {                                                            \
	    len = 1;                                                        \
	    uvc = to_uni_fold( *(U8*)uc, foldbuf, &foldlen );               \
	    foldlen -= UNISKIP( uvc );                                      \
	    uscan = foldbuf + UNISKIP( uvc );                               \
	}                                                                   \
	break;                                                              \
    case trie_utf8:                                                         \
	uvc = utf8n_to_uvuni( (U8*)uc, UTF8_MAXLEN, &len, uniflags );       \
	break;                                                              \
    case trie_plain:                                                        \
	uvc = (UV)*uc;                                                      \
	len = 1;                                                            \
    }                                                                       \
    if (uvc < 256) {                                                        \
	charid = trie->charmap[ uvc ];                                      \
    }                                                                       \
    else {                                                                  \
	charid = 0;                                                         \
	if (widecharmap) {                                                  \
	    SV** const svpp = hv_fetch(widecharmap,                         \
			(char*)&uvc, sizeof(UV), 0);                        \
	    if (svpp)                                                       \
		charid = (U16)SvIV(*svpp);                                  \
	}                                                                   \
    }                                                                       \
} STMT_END

#define REXEC_FBC_EXACTISH_CHECK(CoNd)                 \
{                                                      \
    char *my_strend= (char *)strend;                   \
    if ( (CoNd)                                        \
	 && (ln == len ||                              \
	     !ibcmp_utf8(s, &my_strend, 0,  do_utf8,   \
			m, NULL, ln, (bool)UTF))       \
	 && (!reginfo || regtry(reginfo, &s)) )        \
	goto got_it;                                   \
    else {                                             \
	 U8 foldbuf[UTF8_MAXBYTES_CASE+1];             \
	 uvchr_to_utf8(tmpbuf, c);                     \
	 f = to_utf8_fold(tmpbuf, foldbuf, &foldlen);  \
	 if ( f != c                                   \
	      && (f == c1 || f == c2)                  \
	      && (ln == len ||                         \
	        !ibcmp_utf8(s, &my_strend, 0,  do_utf8,\
			      m, NULL, ln, (bool)UTF)) \
	      && (!reginfo || regtry(reginfo, &s)) )   \
	      goto got_it;                             \
    }                                                  \
}                                                      \
s += len

#define REXEC_FBC_EXACTISH_SCAN(CoNd)                     \
STMT_START {                                              \
    while (s <= e) {                                      \
	if ( (CoNd)                                       \
	     && (ln == 1 || !(OP(c) == EXACTF             \
			      ? ibcmp(s, m, ln)           \
			      : ibcmp_locale(s, m, ln)))  \
	     && (!reginfo || regtry(reginfo, &s)) )        \
	    goto got_it;                                  \
	s++;                                              \
    }                                                     \
} STMT_END

#define REXEC_FBC_UTF8_SCAN(CoDe)                     \
STMT_START {                                          \
    while (s + (uskip = UTF8SKIP(s)) <= strend) {     \
	CoDe                                          \
	s += uskip;                                   \
    }                                                 \
} STMT_END

#define REXEC_FBC_SCAN(CoDe)                          \
STMT_START {                                          \
    while (s < strend) {                              \
	CoDe                                          \
	s++;                                          \
    }                                                 \
} STMT_END

#define REXEC_FBC_UTF8_CLASS_SCAN(CoNd)               \
REXEC_FBC_UTF8_SCAN(                                  \
    if (CoNd) {                                       \
	if (tmp && (!reginfo || regtry(reginfo, &s)))  \
	    goto got_it;                              \
	else                                          \
	    tmp = doevery;                            \
    }                                                 \
    else                                              \
	tmp = 1;                                      \
)

#define REXEC_FBC_CLASS_SCAN(CoNd)                    \
REXEC_FBC_SCAN(                                       \
    if (CoNd) {                                       \
	if (tmp && (!reginfo || regtry(reginfo, &s)))  \
	    goto got_it;                              \
	else                                          \
	    tmp = doevery;                            \
    }                                                 \
    else                                              \
	tmp = 1;                                      \
)

#define REXEC_FBC_TRYIT               \
if ((!reginfo || regtry(reginfo, &s))) \
    goto got_it

#define REXEC_FBC_CSCAN(CoNdUtF8,CoNd)                         \
    if (do_utf8) {                                             \
	REXEC_FBC_UTF8_CLASS_SCAN(CoNdUtF8);                   \
    }                                                          \
    else {                                                     \
	REXEC_FBC_CLASS_SCAN(CoNd);                            \
    }                                                          \
    break
    
#define REXEC_FBC_CSCAN_PRELOAD(UtFpReLoAd,CoNdUtF8,CoNd)      \
    if (do_utf8) {                                             \
	UtFpReLoAd;                                            \
	REXEC_FBC_UTF8_CLASS_SCAN(CoNdUtF8);                   \
    }                                                          \
    else {                                                     \
	REXEC_FBC_CLASS_SCAN(CoNd);                            \
    }                                                          \
    break

#define REXEC_FBC_CSCAN_TAINT(CoNdUtF8,CoNd)                   \
    PL_reg_flags |= RF_tainted;                                \
    if (do_utf8) {                                             \
	REXEC_FBC_UTF8_CLASS_SCAN(CoNdUtF8);                   \
    }                                                          \
    else {                                                     \
	REXEC_FBC_CLASS_SCAN(CoNd);                            \
    }                                                          \
    break

#define DUMP_EXEC_POS(li,s,doutf8) \
    dump_exec_pos(li,s,(PL_regeol),(PL_bostr),(PL_reg_starttry),doutf8)

/* We know what class REx starts with.  Try to find this position... */
/* if reginfo is NULL, its a dryrun */
/* annoyingly all the vars in this routine have different names from their counterparts
   in regmatch. /grrr */

STATIC char *
S_find_byclass(pTHX_ regexp * prog, const regnode *c, char *s, 
    const char *strend, regmatch_info *reginfo)
{
	dVAR;
	const I32 doevery = (prog->intflags & PREGf_SKIP) == 0;
	char *m;
	STRLEN ln;
	STRLEN lnc;
	register STRLEN uskip;
	unsigned int c1;
	unsigned int c2;
	char *e;
	register I32 tmp = 1;	/* Scratch variable? */
	register const bool do_utf8 = PL_reg_match_utf8;
        RXi_GET_DECL(prog,progi);

	PERL_ARGS_ASSERT_FIND_BYCLASS;
        
	/* We know what class it must start with. */
	switch (OP(c)) {
	case ANYOF:
	    if (do_utf8) {
		 REXEC_FBC_UTF8_CLASS_SCAN((ANYOF_FLAGS(c) & ANYOF_UNICODE) ||
			  !UTF8_IS_INVARIANT((U8)s[0]) ?
			  reginclass(prog, c, (U8*)s, 0, do_utf8) :
			  REGINCLASS(prog, c, (U8*)s));
	    }
	    else {
		 while (s < strend) {
		      STRLEN skip = 1;

		      if (REGINCLASS(prog, c, (U8*)s) ||
			  (ANYOF_FOLD_SHARP_S(c, s, strend) &&
			   /* The assignment of 2 is intentional:
			    * for the folded sharp s, the skip is 2. */
			   (skip = SHARP_S_SKIP))) {
			   if (tmp && (!reginfo || regtry(reginfo, &s)))
				goto got_it;
			   else
				tmp = doevery;
		      }
		      else 
			   tmp = 1;
		      s += skip;
		 }
	    }
	    break;
	case CANY:
	    REXEC_FBC_SCAN(
	        if (tmp && (!reginfo || regtry(reginfo, &s)))
		    goto got_it;
		else
		    tmp = doevery;
	    );
	    break;
	case EXACTF:
	    m   = STRING(c);
	    ln  = STR_LEN(c);	/* length to match in octets/bytes */
	    lnc = (I32) ln;	/* length to match in characters */
	    if (UTF) {
	        STRLEN ulen1, ulen2;
		U8 *sm = (U8 *) m;
		U8 tmpbuf1[UTF8_MAXBYTES_CASE+1];
		U8 tmpbuf2[UTF8_MAXBYTES_CASE+1];
		/* used by commented-out code below */
		/*const U32 uniflags = UTF8_ALLOW_DEFAULT;*/
		
                /* XXX: Since the node will be case folded at compile
                   time this logic is a little odd, although im not 
                   sure that its actually wrong. --dmq */
                   
		c1 = to_utf8_lower((U8*)m, tmpbuf1, &ulen1);
		c2 = to_utf8_upper((U8*)m, tmpbuf2, &ulen2);

		/* XXX: This is kinda strange. to_utf8_XYZ returns the 
                   codepoint of the first character in the converted
                   form, yet originally we did the extra step. 
                   No tests fail by commenting this code out however
                   so Ive left it out. -- dmq.
                   
		c1 = utf8n_to_uvchr(tmpbuf1, UTF8_MAXBYTES_CASE, 
				    0, uniflags);
		c2 = utf8n_to_uvchr(tmpbuf2, UTF8_MAXBYTES_CASE,
				    0, uniflags);
                */
                
		lnc = 0;
		while (sm < ((U8 *) m + ln)) {
		    lnc++;
		    sm += UTF8SKIP(sm);
		}
	    }
	    else {
		c1 = *(U8*)m;
		c2 = PL_fold[c1];
	    }
	    goto do_exactf;
	case EXACTFL:
	    m   = STRING(c);
	    ln  = STR_LEN(c);
	    lnc = (I32) ln;
	    c1 = *(U8*)m;
	    c2 = PL_fold_locale[c1];
	  do_exactf:
	    e = HOP3c(strend, -((I32)lnc), s);

	    if (!reginfo && e < s)
		e = s;			/* Due to minlen logic of intuit() */

	    /* The idea in the EXACTF* cases is to first find the
	     * first character of the EXACTF* node and then, if
	     * necessary, case-insensitively compare the full
	     * text of the node.  The c1 and c2 are the first
	     * characters (though in Unicode it gets a bit
	     * more complicated because there are more cases
	     * than just upper and lower: one needs to use
	     * the so-called folding case for case-insensitive
	     * matching (called "loose matching" in Unicode).
	     * ibcmp_utf8() will do just that. */

	    if (do_utf8 || UTF) {
	        UV c, f;
	        U8 tmpbuf [UTF8_MAXBYTES+1];
		STRLEN len = 1;
		STRLEN foldlen;
		const U32 uniflags = UTF8_ALLOW_DEFAULT;
		if (c1 == c2) {
		    /* Upper and lower of 1st char are equal -
		     * probably not a "letter". */
		    while (s <= e) {
		        if (do_utf8) {
		            c = utf8n_to_uvchr((U8*)s, UTF8_MAXBYTES, &len,
					   uniflags);
                        } else {
                            c = *((U8*)s);
                        }					  
			REXEC_FBC_EXACTISH_CHECK(c == c1);
		    }
		}
		else {
		    while (s <= e) {
		        if (do_utf8) {
		            c = utf8n_to_uvchr((U8*)s, UTF8_MAXBYTES, &len,
					   uniflags);
                        } else {
                            c = *((U8*)s);
                        }

			/* Handle some of the three Greek sigmas cases.
			 * Note that not all the possible combinations
			 * are handled here: some of them are handled
			 * by the standard folding rules, and some of
			 * them (the character class or ANYOF cases)
			 * are handled during compiletime in
			 * regexec.c:S_regclass(). */
			if (c == (UV)UNICODE_GREEK_CAPITAL_LETTER_SIGMA ||
			    c == (UV)UNICODE_GREEK_SMALL_LETTER_FINAL_SIGMA)
			    c = (UV)UNICODE_GREEK_SMALL_LETTER_SIGMA;

			REXEC_FBC_EXACTISH_CHECK(c == c1 || c == c2);
		    }
		}
	    }
	    else {
	        /* Neither pattern nor string are UTF8 */
		if (c1 == c2)
		    REXEC_FBC_EXACTISH_SCAN(*(U8*)s == c1);
		else
		    REXEC_FBC_EXACTISH_SCAN(*(U8*)s == c1 || *(U8*)s == c2);
	    }
	    break;
	case BOUNDL:
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
	case BOUND:
	    if (do_utf8) {
		if (s == PL_bostr)
		    tmp = '\n';
		else {
		    U8 * const r = reghop3((U8*)s, -1, (U8*)PL_bostr);
		    tmp = utf8n_to_uvchr(r, UTF8SKIP(r), 0, UTF8_ALLOW_DEFAULT);
		}
		tmp = ((OP(c) == BOUND ?
			isALNUM_uni(tmp) : isALNUM_LC_uvchr(UNI_TO_NATIVE(tmp))) != 0);
		LOAD_UTF8_CHARCLASS_ALNUM();
		REXEC_FBC_UTF8_SCAN(
		    if (tmp == !(OP(c) == BOUND ?
				 (bool)swash_fetch(PL_utf8_alnum, (U8*)s, do_utf8) :
				 isALNUM_LC_utf8((U8*)s)))
		    {
			tmp = !tmp;
			REXEC_FBC_TRYIT;
		}
		);
	    }
	    else {
		tmp = (s != PL_bostr) ? UCHARAT(s - 1) : '\n';
		tmp = ((OP(c) == BOUND ? isALNUM(tmp) : isALNUM_LC(tmp)) != 0);
		REXEC_FBC_SCAN(
		    if (tmp ==
			!(OP(c) == BOUND ? isALNUM(*s) : isALNUM_LC(*s))) {
			tmp = !tmp;
			REXEC_FBC_TRYIT;
		}
		);
	    }
	    if ((!prog->minlen && tmp) && (!reginfo || regtry(reginfo, &s)))
		goto got_it;
	    break;
	case NBOUNDL:
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
	case NBOUND:
	    if (do_utf8) {
		if (s == PL_bostr)
		    tmp = '\n';
		else {
		    U8 * const r = reghop3((U8*)s, -1, (U8*)PL_bostr);
		    tmp = utf8n_to_uvchr(r, UTF8SKIP(r), 0, UTF8_ALLOW_DEFAULT);
		}
		tmp = ((OP(c) == NBOUND ?
			isALNUM_uni(tmp) : isALNUM_LC_uvchr(UNI_TO_NATIVE(tmp))) != 0);
		LOAD_UTF8_CHARCLASS_ALNUM();
		REXEC_FBC_UTF8_SCAN(
		    if (tmp == !(OP(c) == NBOUND ?
				 (bool)swash_fetch(PL_utf8_alnum, (U8*)s, do_utf8) :
				 isALNUM_LC_utf8((U8*)s)))
			tmp = !tmp;
		    else REXEC_FBC_TRYIT;
		);
	    }
	    else {
		tmp = (s != PL_bostr) ? UCHARAT(s - 1) : '\n';
		tmp = ((OP(c) == NBOUND ?
			isALNUM(tmp) : isALNUM_LC(tmp)) != 0);
		REXEC_FBC_SCAN(
		    if (tmp ==
			!(OP(c) == NBOUND ? isALNUM(*s) : isALNUM_LC(*s)))
			tmp = !tmp;
		    else REXEC_FBC_TRYIT;
		);
	    }
	    if ((!prog->minlen && !tmp) && (!reginfo || regtry(reginfo, &s)))
		goto got_it;
	    break;
	case ALNUM:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_PERL_WORD(),
		swash_fetch(RE_utf8_perl_word, (U8*)s, do_utf8),
		isALNUM(*s)
	    );
	case ALNUML:
	    REXEC_FBC_CSCAN_TAINT(
		isALNUM_LC_utf8((U8*)s),
		isALNUM_LC(*s)
	    );
	case NALNUM:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_PERL_WORD(),
		!swash_fetch(RE_utf8_perl_word, (U8*)s, do_utf8),
		!isALNUM(*s)
	    );
	case NALNUML:
	    REXEC_FBC_CSCAN_TAINT(
		!isALNUM_LC_utf8((U8*)s),
		!isALNUM_LC(*s)
	    );
	case SPACE:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_PERL_SPACE(),
		*s == ' ' || swash_fetch(RE_utf8_perl_space,(U8*)s, do_utf8),
		isSPACE(*s)
	    );
	case SPACEL:
	    REXEC_FBC_CSCAN_TAINT(
		*s == ' ' || isSPACE_LC_utf8((U8*)s),
		isSPACE_LC(*s)
	    );
	case NSPACE:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_PERL_SPACE(),
		!(*s == ' ' || swash_fetch(RE_utf8_perl_space,(U8*)s, do_utf8)),
		!isSPACE(*s)
	    );
	case NSPACEL:
	    REXEC_FBC_CSCAN_TAINT(
		!(*s == ' ' || isSPACE_LC_utf8((U8*)s)),
		!isSPACE_LC(*s)
	    );
	case DIGIT:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_POSIX_DIGIT(),
		swash_fetch(RE_utf8_posix_digit,(U8*)s, do_utf8),
		isDIGIT(*s)
	    );
	case DIGITL:
	    REXEC_FBC_CSCAN_TAINT(
		isDIGIT_LC_utf8((U8*)s),
		isDIGIT_LC(*s)
	    );
	case NDIGIT:
	    REXEC_FBC_CSCAN_PRELOAD(
		LOAD_UTF8_CHARCLASS_POSIX_DIGIT(),
		!swash_fetch(RE_utf8_posix_digit,(U8*)s, do_utf8),
		!isDIGIT(*s)
	    );
	case NDIGITL:
	    REXEC_FBC_CSCAN_TAINT(
		!isDIGIT_LC_utf8((U8*)s),
		!isDIGIT_LC(*s)
	    );
	case LNBREAK:
	    REXEC_FBC_CSCAN(
		is_LNBREAK_utf8(s),
		is_LNBREAK_latin1(s)
	    );
	case VERTWS:
	    REXEC_FBC_CSCAN(
		is_VERTWS_utf8(s),
		is_VERTWS_latin1(s)
	    );
	case NVERTWS:
	    REXEC_FBC_CSCAN(
		!is_VERTWS_utf8(s),
		!is_VERTWS_latin1(s)
	    );
	case HORIZWS:
	    REXEC_FBC_CSCAN(
		is_HORIZWS_utf8(s),
		is_HORIZWS_latin1(s)
	    );
	case NHORIZWS:
	    REXEC_FBC_CSCAN(
		!is_HORIZWS_utf8(s),
		!is_HORIZWS_latin1(s)
	    );	    
	case AHOCORASICKC:
	case AHOCORASICK: 
	    {
	        DECL_TRIE_TYPE(c);
                /* what trie are we using right now */
        	reg_ac_data *aho
        	    = (reg_ac_data*)progi->data->data[ ARG( c ) ];
        	reg_trie_data *trie
		    = (reg_trie_data*)progi->data->data[ aho->trie ];
		HV *widecharmap = MUTABLE_HV(progi->data->data[ aho->trie + 1 ]);

		const char *last_start = strend - trie->minlen;
#ifdef DEBUGGING
		const char *real_start = s;
#endif
		STRLEN maxlen = trie->maxlen;
		SV *sv_points;
		U8 **points; /* map of where we were in the input string
		                when reading a given char. For ASCII this
		                is unnecessary overhead as the relationship
		                is always 1:1, but for Unicode, especially
		                case folded Unicode this is not true. */
		U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
		U8 *bitmap=NULL;


                GET_RE_DEBUG_FLAGS_DECL;

                /* We can't just allocate points here. We need to wrap it in
                 * an SV so it gets freed properly if there is a croak while
                 * running the match */
                ENTER;
	        SAVETMPS;
                sv_points=newSV(maxlen * sizeof(U8 *));
                SvCUR_set(sv_points,
                    maxlen * sizeof(U8 *));
                SvPOK_on(sv_points);
                sv_2mortal(sv_points);
                points=(U8**)SvPV_nolen(sv_points );
                if ( trie_type != trie_utf8_fold 
                     && (trie->bitmap || OP(c)==AHOCORASICKC) ) 
                {
                    if (trie->bitmap) 
                        bitmap=(U8*)trie->bitmap;
                    else
                        bitmap=(U8*)ANYOF_BITMAP(c);
                }
                /* this is the Aho-Corasick algorithm modified a touch
                   to include special handling for long "unknown char" 
                   sequences. The basic idea being that we use AC as long
                   as we are dealing with a possible matching char, when
                   we encounter an unknown char (and we have not encountered
                   an accepting state) we scan forward until we find a legal 
                   starting char. 
                   AC matching is basically that of trie matching, except
                   that when we encounter a failing transition, we fall back
                   to the current states "fail state", and try the current char 
                   again, a process we repeat until we reach the root state, 
                   state 1, or a legal transition. If we fail on the root state 
                   then we can either terminate if we have reached an accepting 
                   state previously, or restart the entire process from the beginning 
                   if we have not.

                 */
                while (s <= last_start) {
                    const U32 uniflags = UTF8_ALLOW_DEFAULT;
                    U8 *uc = (U8*)s;
                    U16 charid = 0;
                    U32 base = 1;
                    U32 state = 1;
                    UV uvc = 0;
                    STRLEN len = 0;
                    STRLEN foldlen = 0;
                    U8 *uscan = (U8*)NULL;
                    U8 *leftmost = NULL;
#ifdef DEBUGGING                    
                    U32 accepted_word= 0;
#endif
                    U32 pointpos = 0;

                    while ( state && uc <= (U8*)strend ) {
                        int failed=0;
                        U32 word = aho->states[ state ].wordnum;

                        if( state==1 ) {
                            if ( bitmap ) {
                                DEBUG_TRIE_EXECUTE_r(
                                    if ( uc <= (U8*)last_start && !BITMAP_TEST(bitmap,*uc) ) {
                                        dump_exec_pos( (char *)uc, c, strend, real_start, 
                                            (char *)uc, do_utf8 );
                                        PerlIO_printf( Perl_debug_log,
                                            " Scanning for legal start char...\n");
                                    }
                                );            
                                while ( uc <= (U8*)last_start  && !BITMAP_TEST(bitmap,*uc) ) {
                                    uc++;
                                }
                                s= (char *)uc;
                            }
                            if (uc >(U8*)last_start) break;
                        }
                                            
                        if ( word ) {
                            U8 *lpos= points[ (pointpos - trie->wordlen[word-1] ) % maxlen ];
                            if (!leftmost || lpos < leftmost) {
                                DEBUG_r(accepted_word=word);
                                leftmost= lpos;
                            }
                            if (base==0) break;
                            
                        }
                        points[pointpos++ % maxlen]= uc;
			REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc,
					     uscan, len, uvc, charid, foldlen,
					     foldbuf, uniflags);
                        DEBUG_TRIE_EXECUTE_r({
                            dump_exec_pos( (char *)uc, c, strend, real_start, 
                                s,   do_utf8 );
                            PerlIO_printf(Perl_debug_log,
                                " Charid:%3u CP:%4"UVxf" ",
                                 charid, uvc);
                        });

                        do {
#ifdef DEBUGGING
                            word = aho->states[ state ].wordnum;
#endif
                            base = aho->states[ state ].trans.base;

                            DEBUG_TRIE_EXECUTE_r({
                                if (failed) 
                                    dump_exec_pos( (char *)uc, c, strend, real_start, 
                                        s,   do_utf8 );
                                PerlIO_printf( Perl_debug_log,
                                    "%sState: %4"UVxf", word=%"UVxf,
                                    failed ? " Fail transition to " : "",
                                    (UV)state, (UV)word);
                            });
                            if ( base ) {
                                U32 tmp;
                                if (charid &&
                                     (base + charid > trie->uniquecharcount )
                                     && (base + charid - 1 - trie->uniquecharcount
                                            < trie->lasttrans)
                                     && trie->trans[base + charid - 1 -
                                            trie->uniquecharcount].check == state
                                     && (tmp=trie->trans[base + charid - 1 -
                                        trie->uniquecharcount ].next))
                                {
                                    DEBUG_TRIE_EXECUTE_r(
                                        PerlIO_printf( Perl_debug_log," - legal\n"));
                                    state = tmp;
                                    break;
                                }
                                else {
                                    DEBUG_TRIE_EXECUTE_r(
                                        PerlIO_printf( Perl_debug_log," - fail\n"));
                                    failed = 1;
                                    state = aho->fail[state];
                                }
                            }
                            else {
                                /* we must be accepting here */
                                DEBUG_TRIE_EXECUTE_r(
                                        PerlIO_printf( Perl_debug_log," - accepting\n"));
                                failed = 1;
                                break;
                            }
                        } while(state);
                        uc += len;
                        if (failed) {
                            if (leftmost)
                                break;
                            if (!state) state = 1;
                        }
                    }
                    if ( aho->states[ state ].wordnum ) {
                        U8 *lpos = points[ (pointpos - trie->wordlen[aho->states[ state ].wordnum-1]) % maxlen ];
                        if (!leftmost || lpos < leftmost) {
                            DEBUG_r(accepted_word=aho->states[ state ].wordnum);
                            leftmost = lpos;
                        }
                    }
                    if (leftmost) {
                        s = (char*)leftmost;
                        DEBUG_TRIE_EXECUTE_r({
                            PerlIO_printf( 
                                Perl_debug_log,"Matches word #%"UVxf" at position %"IVdf". Trying full pattern...\n",
                                (UV)accepted_word, (IV)(s - real_start)
                            );
                        });
                        if (!reginfo || regtry(reginfo, &s)) {
                            FREETMPS;
		            LEAVE;
                            goto got_it;
                        }
                        s = HOPc(s,1);
                        DEBUG_TRIE_EXECUTE_r({
                            PerlIO_printf( Perl_debug_log,"Pattern failed. Looking for new start point...\n");
                        });
                    } else {
                        DEBUG_TRIE_EXECUTE_r(
                            PerlIO_printf( Perl_debug_log,"No match.\n"));
                        break;
                    }
                }
                FREETMPS;
                LEAVE;
	    }
	    break;
	default:
	    Perl_croak(aTHX_ "panic: unknown regstclass %d", (int)OP(c));
	    break;
	}
	return 0;
      got_it:
	return s;
}


/*
 - regexec_flags - match a regexp against a string
 */
I32
Perl_regexec_flags(pTHX_ REGEXP * const rx, char *stringarg, register char *strend,
	      char *strbeg, I32 minend, SV *sv, void *data, U32 flags)
/* strend: pointer to null at end of string */
/* strbeg: real beginning of string */
/* minend: end of match must be >=minend after stringarg. */
/* data: May be used for some additional optimizations. 
         Currently its only used, with a U32 cast, for transmitting 
         the ganch offset when doing a /g match. This will change */
/* nosave: For optimizations. */
{
    dVAR;
    struct regexp *const prog = (struct regexp *)SvANY(rx);
    /*register*/ char *s;
    register regnode *c;
    /*register*/ char *startpos = stringarg;
    I32 minlen;		/* must match at least this many chars */
    I32 dontbother = 0;	/* how many characters not to try at end */
    I32 end_shift = 0;			/* Same for the end. */		/* CC */
    I32 scream_pos = -1;		/* Internal iterator of scream. */
    char *scream_olds = NULL;
    const bool do_utf8 = (bool)DO_UTF8(sv);
    I32 multiline;
    RXi_GET_DECL(prog,progi);
    regmatch_info reginfo;  /* create some info to pass to regtry etc */
    regexp_paren_pair *swap = NULL;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGEXEC_FLAGS;
    PERL_UNUSED_ARG(data);

    /* Be paranoid... */
    if (prog == NULL || startpos == NULL) {
	Perl_croak(aTHX_ "NULL regexp parameter");
	return 0;
    }

    multiline = prog->extflags & RXf_PMf_MULTILINE;
    reginfo.prog = rx;	 /* Yes, sorry that this is confusing.  */

    RX_MATCH_UTF8_set(rx, do_utf8);
    DEBUG_EXECUTE_r( 
        debug_start_match(rx, do_utf8, startpos, strend, 
        "Matching");
    );

    minlen = prog->minlen;
    
    if (strend - startpos < (minlen+(prog->check_offset_min<0?prog->check_offset_min:0))) {
        DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
			      "String too short [regexec_flags]...\n"));
	goto phooey;
    }

    
    /* Check validity of program. */
    if (UCHARAT(progi->program) != REG_MAGIC) {
	Perl_croak(aTHX_ "corrupted regexp program");
    }

    PL_reg_flags = 0;
    PL_reg_eval_set = 0;
    PL_reg_maxiter = 0;

    if (RX_UTF8(rx))
	PL_reg_flags |= RF_utf8;

    /* Mark beginning of line for ^ and lookbehind. */
    reginfo.bol = startpos; /* XXX not used ??? */
    PL_bostr  = strbeg;
    reginfo.sv = sv;

    /* Mark end of line for $ (and such) */
    PL_regeol = strend;

    /* see how far we have to get to not match where we matched before */
    reginfo.till = startpos+minend;

    /* If there is a "must appear" string, look for it. */
    s = startpos;

    if (prog->extflags & RXf_GPOS_SEEN) { /* Need to set reginfo->ganch */
	MAGIC *mg;
	if (flags & REXEC_IGNOREPOS){	/* Means: check only at start */
	    reginfo.ganch = startpos + prog->gofs;
	    DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
	      "GPOS IGNOREPOS: reginfo.ganch = startpos + %"UVxf"\n",(UV)prog->gofs));
	} else if (sv && SvTYPE(sv) >= SVt_PVMG
		  && SvMAGIC(sv)
		  && (mg = mg_find(sv, PERL_MAGIC_regex_global))
		  && mg->mg_len >= 0) {
	    reginfo.ganch = strbeg + mg->mg_len;	/* Defined pos() */
	    DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
		"GPOS MAGIC: reginfo.ganch = strbeg + %"IVdf"\n",(IV)mg->mg_len));

	    if (prog->extflags & RXf_ANCH_GPOS) {
	        if (s > reginfo.ganch)
		    goto phooey;
		s = reginfo.ganch - prog->gofs;
	        DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
		     "GPOS ANCH_GPOS: s = ganch - %"UVxf"\n",(UV)prog->gofs));
		if (s < strbeg)
		    goto phooey;
	    }
	}
	else if (data) {
	    reginfo.ganch = strbeg + PTR2UV(data);
            DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
		 "GPOS DATA: reginfo.ganch= strbeg + %"UVxf"\n",PTR2UV(data)));

	} else {				/* pos() not defined */
	    reginfo.ganch = strbeg;
            DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
		 "GPOS: reginfo.ganch = strbeg\n"));
	}
    }
    if (PL_curpm && (PM_GETRE(PL_curpm) == rx)) {
        /* We have to be careful. If the previous successful match
           was from this regex we don't want a subsequent partially
           successful match to clobber the old results.
           So when we detect this possibility we add a swap buffer
           to the re, and switch the buffer each match. If we fail
           we switch it back, otherwise we leave it swapped.
        */
        swap = prog->offs;
        /* do we need a save destructor here for eval dies? */
        Newxz(prog->offs, (prog->nparens + 1), regexp_paren_pair);
    }
    if (!(flags & REXEC_CHECKED) && (prog->check_substr != NULL || prog->check_utf8 != NULL)) {
	re_scream_pos_data d;

	d.scream_olds = &scream_olds;
	d.scream_pos = &scream_pos;
	s = re_intuit_start(rx, sv, s, strend, flags, &d);
	if (!s) {
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Not present...\n"));
	    goto phooey;	/* not present */
	}
    }



    /* Simplest case:  anchored match need be tried only once. */
    /*  [unless only anchor is BOL and multiline is set] */
    if (prog->extflags & (RXf_ANCH & ~RXf_ANCH_GPOS)) {
	if (s == startpos && regtry(&reginfo, &startpos))
	    goto got_it;
	else if (multiline || (prog->intflags & PREGf_IMPLICIT)
		 || (prog->extflags & RXf_ANCH_MBOL)) /* XXXX SBOL? */
	{
	    char *end;

	    if (minlen)
		dontbother = minlen - 1;
	    end = HOP3c(strend, -dontbother, strbeg) - 1;
	    /* for multiline we only have to try after newlines */
	    if (prog->check_substr || prog->check_utf8) {
                /* because of the goto we can not easily reuse the macros for bifurcating the
                   unicode/non-unicode match modes here like we do elsewhere - demerphq */
                if (do_utf8) {
                    if (s == startpos)
                        goto after_try_utf8;
                    while (1) {
                        if (regtry(&reginfo, &s)) {
                            goto got_it;
                        }
                      after_try_utf8:
                        if (s > end) {
                            goto phooey;
                        }
                        if (prog->extflags & RXf_USE_INTUIT) {
                            s = re_intuit_start(rx, sv, s + UTF8SKIP(s), strend, flags, NULL);
                            if (!s) {
                                goto phooey;
                            }
                        }
                        else {
                            s += UTF8SKIP(s);
                        }
                    }
                } /* end search for check string in unicode */
                else {
                    if (s == startpos) {
                        goto after_try_latin;
                    }
                    while (1) {
                        if (regtry(&reginfo, &s)) {
                            goto got_it;
                        }
                      after_try_latin:
                        if (s > end) {
                            goto phooey;
                        }
                        if (prog->extflags & RXf_USE_INTUIT) {
                            s = re_intuit_start(rx, sv, s + 1, strend, flags, NULL);
                            if (!s) {
                                goto phooey;
                            }
                        }
                        else {
                            s++;
                        }
                    }
                } /* end search for check string in latin*/
	    } /* end search for check string */
	    else { /* search for newline */
		if (s > startpos) {
                    /*XXX: The s-- is almost definitely wrong here under unicode - demeprhq*/
		    s--;
		}
                /* We can use a more efficient search as newlines are the same in unicode as they are in latin */
		while (s < end) {
		    if (*s++ == '\n') {	/* don't need PL_utf8skip here */
			if (regtry(&reginfo, &s))
			    goto got_it;
		    }
		}
	    } /* end search for newline */
	} /* end anchored/multiline check string search */
	goto phooey;
    } else if (RXf_GPOS_CHECK == (prog->extflags & RXf_GPOS_CHECK)) 
    {
        /* the warning about reginfo.ganch being used without intialization
           is bogus -- we set it above, when prog->extflags & RXf_GPOS_SEEN 
           and we only enter this block when the same bit is set. */
        char *tmp_s = reginfo.ganch - prog->gofs;

	if (tmp_s >= strbeg && regtry(&reginfo, &tmp_s))
	    goto got_it;
	goto phooey;
    }

    /* Messy cases:  unanchored match. */
    if ((prog->anchored_substr || prog->anchored_utf8) && prog->intflags & PREGf_SKIP) {
	/* we have /x+whatever/ */
	/* it must be a one character string (XXXX Except UTF?) */
	char ch;
#ifdef DEBUGGING
	int did_match = 0;
#endif
	if (!(do_utf8 ? prog->anchored_utf8 : prog->anchored_substr))
	    do_utf8 ? to_utf8_substr(prog) : to_byte_substr(prog);
	ch = SvPVX_const(do_utf8 ? prog->anchored_utf8 : prog->anchored_substr)[0];

	if (do_utf8) {
	    REXEC_FBC_SCAN(
		if (*s == ch) {
		    DEBUG_EXECUTE_r( did_match = 1 );
		    if (regtry(&reginfo, &s)) goto got_it;
		    s += UTF8SKIP(s);
		    while (s < strend && *s == ch)
			s += UTF8SKIP(s);
		}
	    );
	}
	else {
	    REXEC_FBC_SCAN(
		if (*s == ch) {
		    DEBUG_EXECUTE_r( did_match = 1 );
		    if (regtry(&reginfo, &s)) goto got_it;
		    s++;
		    while (s < strend && *s == ch)
			s++;
		}
	    );
	}
	DEBUG_EXECUTE_r(if (!did_match)
		PerlIO_printf(Perl_debug_log,
                                  "Did not find anchored character...\n")
               );
    }
    else if (prog->anchored_substr != NULL
	      || prog->anchored_utf8 != NULL
	      || ((prog->float_substr != NULL || prog->float_utf8 != NULL)
		  && prog->float_max_offset < strend - s)) {
	SV *must;
	I32 back_max;
	I32 back_min;
	char *last;
	char *last1;		/* Last position checked before */
#ifdef DEBUGGING
	int did_match = 0;
#endif
	if (prog->anchored_substr || prog->anchored_utf8) {
	    if (!(do_utf8 ? prog->anchored_utf8 : prog->anchored_substr))
		do_utf8 ? to_utf8_substr(prog) : to_byte_substr(prog);
	    must = do_utf8 ? prog->anchored_utf8 : prog->anchored_substr;
	    back_max = back_min = prog->anchored_offset;
	} else {
	    if (!(do_utf8 ? prog->float_utf8 : prog->float_substr))
		do_utf8 ? to_utf8_substr(prog) : to_byte_substr(prog);
	    must = do_utf8 ? prog->float_utf8 : prog->float_substr;
	    back_max = prog->float_max_offset;
	    back_min = prog->float_min_offset;
	}
	
	    
	if (must == &PL_sv_undef)
	    /* could not downgrade utf8 check substring, so must fail */
	    goto phooey;

        if (back_min<0) {
	    last = strend;
	} else {
            last = HOP3c(strend,	/* Cannot start after this */
        	  -(I32)(CHR_SVLEN(must)
        		 - (SvTAIL(must) != 0) + back_min), strbeg);
        }
	if (s > PL_bostr)
	    last1 = HOPc(s, -1);
	else
	    last1 = s - 1;	/* bogus */

	/* XXXX check_substr already used to find "s", can optimize if
	   check_substr==must. */
	scream_pos = -1;
	dontbother = end_shift;
	strend = HOPc(strend, -dontbother);
	while ( (s <= last) &&
		((flags & REXEC_SCREAM)
		 ? (s = screaminstr(sv, must, HOP3c(s, back_min, (back_min<0 ? strbeg : strend)) - strbeg,
				    end_shift, &scream_pos, 0))
		 : (s = fbm_instr((unsigned char*)HOP3(s, back_min, (back_min<0 ? strbeg : strend)),
				  (unsigned char*)strend, must,
				  multiline ? FBMrf_MULTILINE : 0))) ) {
	    /* we may be pointing at the wrong string */
	    if ((flags & REXEC_SCREAM) && RXp_MATCH_COPIED(prog))
		s = strbeg + (s - SvPVX_const(sv));
	    DEBUG_EXECUTE_r( did_match = 1 );
	    if (HOPc(s, -back_max) > last1) {
		last1 = HOPc(s, -back_min);
		s = HOPc(s, -back_max);
	    }
	    else {
		char * const t = (last1 >= PL_bostr) ? HOPc(last1, 1) : last1 + 1;

		last1 = HOPc(s, -back_min);
		s = t;
	    }
	    if (do_utf8) {
		while (s <= last1) {
		    if (regtry(&reginfo, &s))
			goto got_it;
		    s += UTF8SKIP(s);
		}
	    }
	    else {
		while (s <= last1) {
		    if (regtry(&reginfo, &s))
			goto got_it;
		    s++;
		}
	    }
	}
	DEBUG_EXECUTE_r(if (!did_match) {
            RE_PV_QUOTED_DECL(quoted, do_utf8, PERL_DEBUG_PAD_ZERO(0), 
                SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
            PerlIO_printf(Perl_debug_log, "Did not find %s substr %s%s...\n",
			      ((must == prog->anchored_substr || must == prog->anchored_utf8)
			       ? "anchored" : "floating"),
                quoted, RE_SV_TAIL(must));
        });		    
	goto phooey;
    }
    else if ( (c = progi->regstclass) ) {
	if (minlen) {
	    const OPCODE op = OP(progi->regstclass);
	    /* don't bother with what can't match */
	    if (PL_regkind[op] != EXACT && op != CANY && PL_regkind[op] != TRIE)
	        strend = HOPc(strend, -(minlen - 1));
	}
	DEBUG_EXECUTE_r({
	    SV * const prop = sv_newmortal();
	    regprop(prog, prop, c);
	    {
		RE_PV_QUOTED_DECL(quoted,do_utf8,PERL_DEBUG_PAD_ZERO(1),
		    s,strend-s,60);
		PerlIO_printf(Perl_debug_log,
		    "Matching stclass %.*s against %s (%d chars)\n",
		    (int)SvCUR(prop), SvPVX_const(prop),
		     quoted, (int)(strend - s));
	    }
	});
        if (find_byclass(prog, c, s, strend, &reginfo))
	    goto got_it;
	DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Contradicts stclass... [regexec_flags]\n"));
    }
    else {
	dontbother = 0;
	if (prog->float_substr != NULL || prog->float_utf8 != NULL) {
	    /* Trim the end. */
	    char *last;
	    SV* float_real;

	    if (!(do_utf8 ? prog->float_utf8 : prog->float_substr))
		do_utf8 ? to_utf8_substr(prog) : to_byte_substr(prog);
	    float_real = do_utf8 ? prog->float_utf8 : prog->float_substr;

	    if (flags & REXEC_SCREAM) {
		last = screaminstr(sv, float_real, s - strbeg,
				   end_shift, &scream_pos, 1); /* last one */
		if (!last)
		    last = scream_olds; /* Only one occurrence. */
		/* we may be pointing at the wrong string */
		else if (RXp_MATCH_COPIED(prog))
		    s = strbeg + (s - SvPVX_const(sv));
	    }
	    else {
		STRLEN len;
                const char * const little = SvPV_const(float_real, len);

		if (SvTAIL(float_real)) {
		    if (memEQ(strend - len + 1, little, len - 1))
			last = strend - len + 1;
		    else if (!multiline)
			last = memEQ(strend - len, little, len)
			    ? strend - len : NULL;
		    else
			goto find_last;
		} else {
		  find_last:
		    if (len)
			last = rninstr(s, strend, little, little + len);
		    else
			last = strend;	/* matching "$" */
		}
	    }
	    if (last == NULL) {
		DEBUG_EXECUTE_r(
		    PerlIO_printf(Perl_debug_log,
			"%sCan't trim the tail, match fails (should not happen)%s\n",
	                PL_colors[4], PL_colors[5]));
		goto phooey; /* Should not happen! */
	    }
	    dontbother = strend - last + prog->float_min_offset;
	}
	if (minlen && (dontbother < minlen))
	    dontbother = minlen - 1;
	strend -= dontbother; 		   /* this one's always in bytes! */
	/* We don't know much -- general case. */
	if (do_utf8) {
	    for (;;) {
		if (regtry(&reginfo, &s))
		    goto got_it;
		if (s >= strend)
		    break;
		s += UTF8SKIP(s);
	    };
	}
	else {
	    do {
		if (regtry(&reginfo, &s))
		    goto got_it;
	    } while (s++ < strend);
	}
    }

    /* Failure. */
    goto phooey;

got_it:
    Safefree(swap);
    RX_MATCH_TAINTED_set(rx, PL_reg_flags & RF_tainted);

    if (PL_reg_eval_set)
	restore_pos(aTHX_ prog);
    if (RXp_PAREN_NAMES(prog)) 
        (void)hv_iterinit(RXp_PAREN_NAMES(prog));

    /* make sure $`, $&, $', and $digit will work later */
    if ( !(flags & REXEC_NOT_FIRST) ) {
	RX_MATCH_COPY_FREE(rx);
	if (flags & REXEC_COPY_STR) {
	    const I32 i = PL_regeol - startpos + (stringarg - strbeg);
#ifdef PERL_OLD_COPY_ON_WRITE
	    if ((SvIsCOW(sv)
		 || (SvFLAGS(sv) & CAN_COW_MASK) == CAN_COW_FLAGS)) {
		if (DEBUG_C_TEST) {
		    PerlIO_printf(Perl_debug_log,
				  "Copy on write: regexp capture, type %d\n",
				  (int) SvTYPE(sv));
		}
		prog->saved_copy = sv_setsv_cow(prog->saved_copy, sv);
		prog->subbeg = (char *)SvPVX_const(prog->saved_copy);
		assert (SvPOKp(prog->saved_copy));
	    } else
#endif
	    {
		RX_MATCH_COPIED_on(rx);
		s = savepvn(strbeg, i);
		prog->subbeg = s;
	    }
	    prog->sublen = i;
	}
	else {
	    prog->subbeg = strbeg;
	    prog->sublen = PL_regeol - strbeg;	/* strend may have been modified */
	}
    }

    return 1;

phooey:
    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%sMatch failed%s\n",
			  PL_colors[4], PL_colors[5]));
    if (PL_reg_eval_set)
	restore_pos(aTHX_ prog);
    if (swap) {
        /* we failed :-( roll it back */
        Safefree(prog->offs);
        prog->offs = swap;
    }

    return 0;
}


/*
 - regtry - try match at specific point
 */
STATIC I32			/* 0 failure, 1 success */
S_regtry(pTHX_ regmatch_info *reginfo, char **startpos)
{
    dVAR;
    CHECKPOINT lastcp;
    REGEXP *const rx = reginfo->prog;
    regexp *const prog = (struct regexp *)SvANY(rx);
    RXi_GET_DECL(prog,progi);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGTRY;

    reginfo->cutpoint=NULL;

    if ((prog->extflags & RXf_EVAL_SEEN) && !PL_reg_eval_set) {
	MAGIC *mg;

	PL_reg_eval_set = RS_init;
	DEBUG_EXECUTE_r(DEBUG_s(
	    PerlIO_printf(Perl_debug_log, "  setting stack tmpbase at %"IVdf"\n",
			  (IV)(PL_stack_sp - PL_stack_base));
	    ));
	SAVESTACK_CXPOS();
	cxstack[cxstack_ix].blk_oldsp = PL_stack_sp - PL_stack_base;
	/* Otherwise OP_NEXTSTATE will free whatever on stack now.  */
	SAVETMPS;
	/* Apparently this is not needed, judging by wantarray. */
	/* SAVEI8(cxstack[cxstack_ix].blk_gimme);
	   cxstack[cxstack_ix].blk_gimme = G_SCALAR; */

	if (reginfo->sv) {
	    /* Make $_ available to executed code. */
	    if (reginfo->sv != DEFSV) {
		SAVE_DEFSV;
		DEFSV_set(reginfo->sv);
	    }
	
	    if (!(SvTYPE(reginfo->sv) >= SVt_PVMG && SvMAGIC(reginfo->sv)
		  && (mg = mg_find(reginfo->sv, PERL_MAGIC_regex_global)))) {
		/* prepare for quick setting of pos */
#ifdef PERL_OLD_COPY_ON_WRITE
		if (SvIsCOW(reginfo->sv))
		    sv_force_normal_flags(reginfo->sv, 0);
#endif
		mg = sv_magicext(reginfo->sv, NULL, PERL_MAGIC_regex_global,
				 &PL_vtbl_mglob, NULL, 0);
		mg->mg_len = -1;
	    }
	    PL_reg_magic    = mg;
	    PL_reg_oldpos   = mg->mg_len;
	    SAVEDESTRUCTOR_X(restore_pos, prog);
        }
        if (!PL_reg_curpm) {
	    Newxz(PL_reg_curpm, 1, PMOP);
#ifdef USE_ITHREADS
            {
		SV* const repointer = &PL_sv_undef;
                /* this regexp is also owned by the new PL_reg_curpm, which
		   will try to free it.  */
                av_push(PL_regex_padav, repointer);
                PL_reg_curpm->op_pmoffset = av_len(PL_regex_padav);
                PL_regex_pad = AvARRAY(PL_regex_padav);
            }
#endif      
        }
#ifdef USE_ITHREADS
	/* It seems that non-ithreads works both with and without this code.
	   So for efficiency reasons it seems best not to have the code
	   compiled when it is not needed.  */
	/* This is safe against NULLs: */
	ReREFCNT_dec(PM_GETRE(PL_reg_curpm));
	/* PM_reg_curpm owns a reference to this regexp.  */
	ReREFCNT_inc(rx);
#endif
	PM_SETRE(PL_reg_curpm, rx);
	PL_reg_oldcurpm = PL_curpm;
	PL_curpm = PL_reg_curpm;
	if (RXp_MATCH_COPIED(prog)) {
	    /*  Here is a serious problem: we cannot rewrite subbeg,
		since it may be needed if this match fails.  Thus
		$` inside (?{}) could fail... */
	    PL_reg_oldsaved = prog->subbeg;
	    PL_reg_oldsavedlen = prog->sublen;
#ifdef PERL_OLD_COPY_ON_WRITE
	    PL_nrs = prog->saved_copy;
#endif
	    RXp_MATCH_COPIED_off(prog);
	}
	else
	    PL_reg_oldsaved = NULL;
	prog->subbeg = PL_bostr;
	prog->sublen = PL_regeol - PL_bostr; /* strend may have been modified */
    }
    DEBUG_EXECUTE_r(PL_reg_starttry = *startpos);
    prog->offs[0].start = *startpos - PL_bostr;
    PL_reginput = *startpos;
    PL_reglastparen = &prog->lastparen;
    PL_reglastcloseparen = &prog->lastcloseparen;
    prog->lastparen = 0;
    prog->lastcloseparen = 0;
    PL_regsize = 0;
    PL_regoffs = prog->offs;
    if (PL_reg_start_tmpl <= prog->nparens) {
	PL_reg_start_tmpl = prog->nparens*3/2 + 3;
        if(PL_reg_start_tmp)
            Renew(PL_reg_start_tmp, PL_reg_start_tmpl, char*);
        else
            Newx(PL_reg_start_tmp, PL_reg_start_tmpl, char*);
    }

    /* XXXX What this code is doing here?!!!  There should be no need
       to do this again and again, PL_reglastparen should take care of
       this!  --ilya*/

    /* Tests pat.t#187 and split.t#{13,14} seem to depend on this code.
     * Actually, the code in regcppop() (which Ilya may be meaning by
     * PL_reglastparen), is not needed at all by the test suite
     * (op/regexp, op/pat, op/split), but that code is needed otherwise
     * this erroneously leaves $1 defined: "1" =~ /^(?:(\d)x)?\d$/
     * Meanwhile, this code *is* needed for the
     * above-mentioned test suite tests to succeed.  The common theme
     * on those tests seems to be returning null fields from matches.
     * --jhi updated by dapm */
#if 1
    if (prog->nparens) {
	regexp_paren_pair *pp = PL_regoffs;
	register I32 i;
	for (i = prog->nparens; i > (I32)*PL_reglastparen; i--) {
	    ++pp;
	    pp->start = -1;
	    pp->end = -1;
	}
    }
#endif
    REGCP_SET(lastcp);
    if (regmatch(reginfo, progi->program + 1)) {
	PL_regoffs[0].end = PL_reginput - PL_bostr;
	return 1;
    }
    if (reginfo->cutpoint)
        *startpos= reginfo->cutpoint;
    REGCP_UNWIND(lastcp);
    return 0;
}


#define sayYES goto yes
#define sayNO goto no
#define sayNO_SILENT goto no_silent

/* we dont use STMT_START/END here because it leads to 
   "unreachable code" warnings, which are bogus, but distracting. */
#define CACHEsayNO \
    if (ST.cache_mask) \
       PL_reg_poscache[ST.cache_offset] |= ST.cache_mask; \
    sayNO

/* this is used to determine how far from the left messages like
   'failed...' are printed. It should be set such that messages 
   are inline with the regop output that created them.
*/
#define REPORT_CODE_OFF 32


/* Make sure there is a test for this +1 options in re_tests */
#define TRIE_INITAL_ACCEPT_BUFFLEN 4;

#define CHRTEST_UNINIT -1001 /* c1/c2 haven't been calculated yet */
#define CHRTEST_VOID   -1000 /* the c1/c2 "next char" test should be skipped */

#define SLAB_FIRST(s) (&(s)->states[0])
#define SLAB_LAST(s)  (&(s)->states[PERL_REGMATCH_SLAB_SLOTS-1])

/* grab a new slab and return the first slot in it */

STATIC regmatch_state *
S_push_slab(pTHX)
{
#if PERL_VERSION < 9 && !defined(PERL_CORE)
    dMY_CXT;
#endif
    regmatch_slab *s = PL_regmatch_slab->next;
    if (!s) {
	Newx(s, 1, regmatch_slab);
	s->prev = PL_regmatch_slab;
	s->next = NULL;
	PL_regmatch_slab->next = s;
    }
    PL_regmatch_slab = s;
    return SLAB_FIRST(s);
}


/* push a new state then goto it */

#define PUSH_STATE_GOTO(state, node) \
    scan = node; \
    st->resume_state = state; \
    goto push_state;

/* push a new state with success backtracking, then goto it */

#define PUSH_YES_STATE_GOTO(state, node) \
    scan = node; \
    st->resume_state = state; \
    goto push_yes_state;



/*

regmatch() - main matching routine

This is basically one big switch statement in a loop. We execute an op,
set 'next' to point the next op, and continue. If we come to a point which
we may need to backtrack to on failure such as (A|B|C), we push a
backtrack state onto the backtrack stack. On failure, we pop the top
state, and re-enter the loop at the state indicated. If there are no more
states to pop, we return failure.

Sometimes we also need to backtrack on success; for example /A+/, where
after successfully matching one A, we need to go back and try to
match another one; similarly for lookahead assertions: if the assertion
completes successfully, we backtrack to the state just before the assertion
and then carry on.  In these cases, the pushed state is marked as
'backtrack on success too'. This marking is in fact done by a chain of
pointers, each pointing to the previous 'yes' state. On success, we pop to
the nearest yes state, discarding any intermediate failure-only states.
Sometimes a yes state is pushed just to force some cleanup code to be
called at the end of a successful match or submatch; e.g. (??{$re}) uses
it to free the inner regex.

Note that failure backtracking rewinds the cursor position, while
success backtracking leaves it alone.

A pattern is complete when the END op is executed, while a subpattern
such as (?=foo) is complete when the SUCCESS op is executed. Both of these
ops trigger the "pop to last yes state if any, otherwise return true"
behaviour.

A common convention in this function is to use A and B to refer to the two
subpatterns (or to the first nodes thereof) in patterns like /A*B/: so A is
the subpattern to be matched possibly multiple times, while B is the entire
rest of the pattern. Variable and state names reflect this convention.

The states in the main switch are the union of ops and failure/success of
substates associated with with that op.  For example, IFMATCH is the op
that does lookahead assertions /(?=A)B/ and so the IFMATCH state means
'execute IFMATCH'; while IFMATCH_A is a state saying that we have just
successfully matched A and IFMATCH_A_fail is a state saying that we have
just failed to match A. Resume states always come in pairs. The backtrack
state we push is marked as 'IFMATCH_A', but when that is popped, we resume
at IFMATCH_A or IFMATCH_A_fail, depending on whether we are backtracking
on success or failure.

The struct that holds a backtracking state is actually a big union, with
one variant for each major type of op. The variable st points to the
top-most backtrack struct. To make the code clearer, within each
block of code we #define ST to alias the relevant union.

Here's a concrete example of a (vastly oversimplified) IFMATCH
implementation:

    switch (state) {
    ....

#define ST st->u.ifmatch

    case IFMATCH: // we are executing the IFMATCH op, (?=A)B
	ST.foo = ...; // some state we wish to save
	...
	// push a yes backtrack state with a resume value of
	// IFMATCH_A/IFMATCH_A_fail, then continue execution at the
	// first node of A:
	PUSH_YES_STATE_GOTO(IFMATCH_A, A);
	// NOTREACHED

    case IFMATCH_A: // we have successfully executed A; now continue with B
	next = B;
	bar = ST.foo; // do something with the preserved value
	break;

    case IFMATCH_A_fail: // A failed, so the assertion failed
	...;   // do some housekeeping, then ...
	sayNO; // propagate the failure

#undef ST

    ...
    }

For any old-timers reading this who are familiar with the old recursive
approach, the code above is equivalent to:

    case IFMATCH: // we are executing the IFMATCH op, (?=A)B
    {
	int foo = ...
	...
	if (regmatch(A)) {
	    next = B;
	    bar = foo;
	    break;
	}
	...;   // do some housekeeping, then ...
	sayNO; // propagate the failure
    }

The topmost backtrack state, pointed to by st, is usually free. If you
want to claim it, populate any ST.foo fields in it with values you wish to
save, then do one of

	PUSH_STATE_GOTO(resume_state, node);
	PUSH_YES_STATE_GOTO(resume_state, node);

which sets that backtrack state's resume value to 'resume_state', pushes a
new free entry to the top of the backtrack stack, then goes to 'node'.
On backtracking, the free slot is popped, and the saved state becomes the
new free state. An ST.foo field in this new top state can be temporarily
accessed to retrieve values, but once the main loop is re-entered, it
becomes available for reuse.

Note that the depth of the backtrack stack constantly increases during the
left-to-right execution of the pattern, rather than going up and down with
the pattern nesting. For example the stack is at its maximum at Z at the
end of the pattern, rather than at X in the following:

    /(((X)+)+)+....(Y)+....Z/

The only exceptions to this are lookahead/behind assertions and the cut,
(?>A), which pop all the backtrack states associated with A before
continuing.
 
Bascktrack state structs are allocated in slabs of about 4K in size.
PL_regmatch_state and st always point to the currently active state,
and PL_regmatch_slab points to the slab currently containing
PL_regmatch_state.  The first time regmatch() is called, the first slab is
allocated, and is never freed until interpreter destruction. When the slab
is full, a new one is allocated and chained to the end. At exit from
regmatch(), slabs allocated since entry are freed.

*/
 

#define DEBUG_STATE_pp(pp)				    \
    DEBUG_STATE_r({					    \
	DUMP_EXEC_POS(locinput, scan, do_utf8);		    \
	PerlIO_printf(Perl_debug_log,			    \
	    "    %*s"pp" %s%s%s%s%s\n",			    \
	    depth*2, "",				    \
	    PL_reg_name[st->resume_state],                     \
	    ((st==yes_state||st==mark_state) ? "[" : ""),   \
	    ((st==yes_state) ? "Y" : ""),                   \
	    ((st==mark_state) ? "M" : ""),                  \
	    ((st==yes_state||st==mark_state) ? "]" : "")    \
	);                                                  \
    });


#define REG_NODE_NUM(x) ((x) ? (int)((x)-prog) : -1)

#ifdef DEBUGGING

STATIC void
S_debug_start_match(pTHX_ const REGEXP *prog, const bool do_utf8, 
    const char *start, const char *end, const char *blurb)
{
    const bool utf8_pat = RX_UTF8(prog) ? 1 : 0;

    PERL_ARGS_ASSERT_DEBUG_START_MATCH;

    if (!PL_colorset)   
            reginitcolors();    
    {
        RE_PV_QUOTED_DECL(s0, utf8_pat, PERL_DEBUG_PAD_ZERO(0), 
            RX_PRECOMP_const(prog), RX_PRELEN(prog), 60);   
        
        RE_PV_QUOTED_DECL(s1, do_utf8, PERL_DEBUG_PAD_ZERO(1), 
            start, end - start, 60); 
        
        PerlIO_printf(Perl_debug_log, 
            "%s%s REx%s %s against %s\n", 
		       PL_colors[4], blurb, PL_colors[5], s0, s1); 
        
        if (do_utf8||utf8_pat) 
            PerlIO_printf(Perl_debug_log, "UTF-8 %s%s%s...\n",
                utf8_pat ? "pattern" : "",
                utf8_pat && do_utf8 ? " and " : "",
                do_utf8 ? "string" : ""
            ); 
    }
}

STATIC void
S_dump_exec_pos(pTHX_ const char *locinput, 
                      const regnode *scan, 
                      const char *loc_regeol, 
                      const char *loc_bostr, 
                      const char *loc_reg_starttry,
                      const bool do_utf8)
{
    const int docolor = *PL_colors[0] || *PL_colors[2] || *PL_colors[4];
    const int taill = (docolor ? 10 : 7); /* 3 chars for "> <" */
    int l = (loc_regeol - locinput) > taill ? taill : (loc_regeol - locinput);
    /* The part of the string before starttry has one color
       (pref0_len chars), between starttry and current
       position another one (pref_len - pref0_len chars),
       after the current position the third one.
       We assume that pref0_len <= pref_len, otherwise we
       decrease pref0_len.  */
    int pref_len = (locinput - loc_bostr) > (5 + taill) - l
	? (5 + taill) - l : locinput - loc_bostr;
    int pref0_len;

    PERL_ARGS_ASSERT_DUMP_EXEC_POS;

    while (do_utf8 && UTF8_IS_CONTINUATION(*(U8*)(locinput - pref_len)))
	pref_len++;
    pref0_len = pref_len  - (locinput - loc_reg_starttry);
    if (l + pref_len < (5 + taill) && l < loc_regeol - locinput)
	l = ( loc_regeol - locinput > (5 + taill) - pref_len
	      ? (5 + taill) - pref_len : loc_regeol - locinput);
    while (do_utf8 && UTF8_IS_CONTINUATION(*(U8*)(locinput + l)))
	l--;
    if (pref0_len < 0)
	pref0_len = 0;
    if (pref0_len > pref_len)
	pref0_len = pref_len;
    {
	const int is_uni = (do_utf8 && OP(scan) != CANY) ? 1 : 0;

	RE_PV_COLOR_DECL(s0,len0,is_uni,PERL_DEBUG_PAD(0),
	    (locinput - pref_len),pref0_len, 60, 4, 5);
	
	RE_PV_COLOR_DECL(s1,len1,is_uni,PERL_DEBUG_PAD(1),
		    (locinput - pref_len + pref0_len),
		    pref_len - pref0_len, 60, 2, 3);
	
	RE_PV_COLOR_DECL(s2,len2,is_uni,PERL_DEBUG_PAD(2),
		    locinput, loc_regeol - locinput, 10, 0, 1);

	const STRLEN tlen=len0+len1+len2;
	PerlIO_printf(Perl_debug_log,
		    "%4"IVdf" <%.*s%.*s%s%.*s>%*s|",
		    (IV)(locinput - loc_bostr),
		    len0, s0,
		    len1, s1,
		    (docolor ? "" : "> <"),
		    len2, s2,
		    (int)(tlen > 19 ? 0 :  19 - tlen),
		    "");
    }
}

#endif

/* reg_check_named_buff_matched()
 * Checks to see if a named buffer has matched. The data array of 
 * buffer numbers corresponding to the buffer is expected to reside
 * in the regexp->data->data array in the slot stored in the ARG() of
 * node involved. Note that this routine doesn't actually care about the
 * name, that information is not preserved from compilation to execution.
 * Returns the index of the leftmost defined buffer with the given name
 * or 0 if non of the buffers matched.
 */
STATIC I32
S_reg_check_named_buff_matched(pTHX_ const regexp *rex, const regnode *scan)
{
    I32 n;
    RXi_GET_DECL(rex,rexi);
    SV *sv_dat= MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
    I32 *nums=(I32*)SvPVX(sv_dat);

    PERL_ARGS_ASSERT_REG_CHECK_NAMED_BUFF_MATCHED;

    for ( n=0; n<SvIVX(sv_dat); n++ ) {
        if ((I32)*PL_reglastparen >= nums[n] &&
            PL_regoffs[nums[n]].end != -1)
        {
            return nums[n];
        }
    }
    return 0;
}


/* free all slabs above current one  - called during LEAVE_SCOPE */

STATIC void
S_clear_backtrack_stack(pTHX_ void *p)
{
    regmatch_slab *s = PL_regmatch_slab->next;
    PERL_UNUSED_ARG(p);

    if (!s)
	return;
    PL_regmatch_slab->next = NULL;
    while (s) {
	regmatch_slab * const osl = s;
	s = s->next;
	Safefree(osl);
    }
}


#define SETREX(Re1,Re2) \
    if (PL_reg_eval_set) PM_SETRE((PL_reg_curpm), (Re2)); \
    Re1 = (Re2)

STATIC I32			/* 0 failure, 1 success */
S_regmatch(pTHX_ regmatch_info *reginfo, regnode *prog)
{
#if PERL_VERSION < 9 && !defined(PERL_CORE)
    dMY_CXT;
#endif
    dVAR;
    register const bool do_utf8 = PL_reg_match_utf8;
    const U32 uniflags = UTF8_ALLOW_DEFAULT;
    REGEXP *rex_sv = reginfo->prog;
    regexp *rex = (struct regexp *)SvANY(rex_sv);
    RXi_GET_DECL(rex,rexi);
    I32	oldsave;
    /* the current state. This is a cached copy of PL_regmatch_state */
    register regmatch_state *st;
    /* cache heavy used fields of st in registers */
    register regnode *scan;
    register regnode *next;
    register U32 n = 0;	/* general value; init to avoid compiler warning */
    register I32 ln = 0; /* len or last;  init to avoid compiler warning */
    register char *locinput = PL_reginput;
    register I32 nextchr;   /* is always set to UCHARAT(locinput) */

    bool result = 0;	    /* return value of S_regmatch */
    int depth = 0;	    /* depth of backtrack stack */
    U32 nochange_depth = 0; /* depth of GOSUB recursion with nochange */
    const U32 max_nochange_depth =
        (3 * rex->nparens > MAX_RECURSE_EVAL_NOCHANGE_DEPTH) ?
        3 * rex->nparens : MAX_RECURSE_EVAL_NOCHANGE_DEPTH;
    regmatch_state *yes_state = NULL; /* state to pop to on success of
							    subpattern */
    /* mark_state piggy backs on the yes_state logic so that when we unwind 
       the stack on success we can update the mark_state as we go */
    regmatch_state *mark_state = NULL; /* last mark state we have seen */
    regmatch_state *cur_eval = NULL; /* most recent EVAL_AB state */
    struct regmatch_state  *cur_curlyx = NULL; /* most recent curlyx */
    U32 state_num;
    bool no_final = 0;      /* prevent failure from backtracking? */
    bool do_cutgroup = 0;   /* no_final only until next branch/trie entry */
    char *startpoint = PL_reginput;
    SV *popmark = NULL;     /* are we looking for a mark? */
    SV *sv_commit = NULL;   /* last mark name seen in failure */
    SV *sv_yes_mark = NULL; /* last mark name we have seen 
                               during a successfull match */
    U32 lastopen = 0;       /* last open we saw */
    bool has_cutgroup = RX_HAS_CUTGROUP(rex) ? 1 : 0;   
    SV* const oreplsv = GvSV(PL_replgv);
    /* these three flags are set by various ops to signal information to
     * the very next op. They have a useful lifetime of exactly one loop
     * iteration, and are not preserved or restored by state pushes/pops
     */
    bool sw = 0;	    /* the condition value in (?(cond)a|b) */
    bool minmod = 0;	    /* the next "{n,m}" is a "{n,m}?" */
    int logical = 0;	    /* the following EVAL is:
				0: (?{...})
				1: (?(?{...})X|Y)
				2: (??{...})
			       or the following IFMATCH/UNLESSM is:
			        false: plain (?=foo)
				true:  used as a condition: (?(?=foo))
			    */
#ifdef DEBUGGING
    GET_RE_DEBUG_FLAGS_DECL;
#endif

    PERL_ARGS_ASSERT_REGMATCH;

    DEBUG_OPTIMISE_r( DEBUG_EXECUTE_r({
	    PerlIO_printf(Perl_debug_log,"regmatch start\n");
    }));
    /* on first ever call to regmatch, allocate first slab */
    if (!PL_regmatch_slab) {
	Newx(PL_regmatch_slab, 1, regmatch_slab);
	PL_regmatch_slab->prev = NULL;
	PL_regmatch_slab->next = NULL;
	PL_regmatch_state = SLAB_FIRST(PL_regmatch_slab);
    }

    oldsave = PL_savestack_ix;
    SAVEDESTRUCTOR_X(S_clear_backtrack_stack, NULL);
    SAVEVPTR(PL_regmatch_slab);
    SAVEVPTR(PL_regmatch_state);

    /* grab next free state slot */
    st = ++PL_regmatch_state;
    if (st >  SLAB_LAST(PL_regmatch_slab))
	st = PL_regmatch_state = S_push_slab(aTHX);

    /* Note that nextchr is a byte even in UTF */
    nextchr = UCHARAT(locinput);
    scan = prog;
    while (scan != NULL) {

        DEBUG_EXECUTE_r( {
	    SV * const prop = sv_newmortal();
	    regnode *rnext=regnext(scan);
	    DUMP_EXEC_POS( locinput, scan, do_utf8 );
	    regprop(rex, prop, scan);
            
	    PerlIO_printf(Perl_debug_log,
		    "%3"IVdf":%*s%s(%"IVdf")\n",
		    (IV)(scan - rexi->program), depth*2, "",
		    SvPVX_const(prop),
		    (PL_regkind[OP(scan)] == END || !rnext) ? 
		        0 : (IV)(rnext - rexi->program));
	});

	next = scan + NEXT_OFF(scan);
	if (next == scan)
	    next = NULL;
	state_num = OP(scan);

      reenter_switch:

	assert(PL_reglastparen == &rex->lastparen);
	assert(PL_reglastcloseparen == &rex->lastcloseparen);
	assert(PL_regoffs == rex->offs);

	switch (state_num) {
	case BOL:
	    if (locinput == PL_bostr)
	    {
		/* reginfo->till = reginfo->bol; */
		break;
	    }
	    sayNO;
	case MBOL:
	    if (locinput == PL_bostr ||
		((nextchr || locinput < PL_regeol) && locinput[-1] == '\n'))
	    {
		break;
	    }
	    sayNO;
	case SBOL:
	    if (locinput == PL_bostr)
		break;
	    sayNO;
	case GPOS:
	    if (locinput == reginfo->ganch)
		break;
	    sayNO;

	case KEEPS:
	    /* update the startpoint */
	    st->u.keeper.val = PL_regoffs[0].start;
	    PL_reginput = locinput;
	    PL_regoffs[0].start = locinput - PL_bostr;
	    PUSH_STATE_GOTO(KEEPS_next, next);
	    /*NOT-REACHED*/
	case KEEPS_next_fail:
	    /* rollback the start point change */
	    PL_regoffs[0].start = st->u.keeper.val;
	    sayNO_SILENT;
	    /*NOT-REACHED*/
	case EOL:
		goto seol;
	case MEOL:
	    if ((nextchr || locinput < PL_regeol) && nextchr != '\n')
		sayNO;
	    break;
	case SEOL:
	  seol:
	    if ((nextchr || locinput < PL_regeol) && nextchr != '\n')
		sayNO;
	    if (PL_regeol - locinput > 1)
		sayNO;
	    break;
	case EOS:
	    if (PL_regeol != locinput)
		sayNO;
	    break;
	case SANY:
	    if (!nextchr && locinput >= PL_regeol)
		sayNO;
 	    if (do_utf8) {
	        locinput += PL_utf8skip[nextchr];
		if (locinput > PL_regeol)
 		    sayNO;
 		nextchr = UCHARAT(locinput);
 	    }
 	    else
 		nextchr = UCHARAT(++locinput);
	    break;
	case CANY:
	    if (!nextchr && locinput >= PL_regeol)
		sayNO;
	    nextchr = UCHARAT(++locinput);
	    break;
	case REG_ANY:
	    if ((!nextchr && locinput >= PL_regeol) || nextchr == '\n')
		sayNO;
	    if (do_utf8) {
		locinput += PL_utf8skip[nextchr];
		if (locinput > PL_regeol)
		    sayNO;
		nextchr = UCHARAT(locinput);
	    }
	    else
		nextchr = UCHARAT(++locinput);
	    break;

#undef  ST
#define ST st->u.trie
        case TRIEC:
            /* In this case the charclass data is available inline so
               we can fail fast without a lot of extra overhead. 
             */
            if (scan->flags == EXACT || !do_utf8) {
                if(!ANYOF_BITMAP_TEST(scan, *locinput)) {
                    DEBUG_EXECUTE_r(
                        PerlIO_printf(Perl_debug_log,
                    	          "%*s  %sfailed to match trie start class...%s\n",
                    	          REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                    );
                    sayNO_SILENT;
                    /* NOTREACHED */
                }        	        
            }
            /* FALL THROUGH */
	case TRIE:
	    {
                /* what type of TRIE am I? (utf8 makes this contextual) */
                DECL_TRIE_TYPE(scan);

                /* what trie are we using right now */
		reg_trie_data * const trie
        	    = (reg_trie_data*)rexi->data->data[ ARG( scan ) ];
		HV * widecharmap = MUTABLE_HV(rexi->data->data[ ARG( scan ) + 1 ]);
                U32 state = trie->startstate;

        	if (trie->bitmap && trie_type != trie_utf8_fold &&
        	    !TRIE_BITMAP_TEST(trie,*locinput)
        	) {
        	    if (trie->states[ state ].wordnum) {
        	         DEBUG_EXECUTE_r(
                            PerlIO_printf(Perl_debug_log,
                        	          "%*s  %smatched empty string...%s\n",
                        	          REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                        );
        	        break;
        	    } else {
        	        DEBUG_EXECUTE_r(
                            PerlIO_printf(Perl_debug_log,
                        	          "%*s  %sfailed to match trie start class...%s\n",
                        	          REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                        );
        	        sayNO_SILENT;
        	   }
                }

            { 
		U8 *uc = ( U8* )locinput;

		STRLEN len = 0;
		STRLEN foldlen = 0;
		U8 *uscan = (U8*)NULL;
		STRLEN bufflen=0;
		SV *sv_accept_buff = NULL;
		U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];

	    	ST.accepted = 0; /* how many accepting states we have seen */
		ST.B = next;
		ST.jump = trie->jump;
		ST.me = scan;
	        /*
        	   traverse the TRIE keeping track of all accepting states
        	   we transition through until we get to a failing node.
        	*/

		while ( state && uc <= (U8*)PL_regeol ) {
                    U32 base = trie->states[ state ].trans.base;
                    UV uvc = 0;
                    U16 charid;
                    /* We use charid to hold the wordnum as we don't use it
                       for charid until after we have done the wordnum logic. 
                       We define an alias just so that the wordnum logic reads
                       more naturally. */

#define got_wordnum charid
                    got_wordnum = trie->states[ state ].wordnum;

		    if ( got_wordnum ) {
			if ( ! ST.accepted ) {
			    ENTER;
			    SAVETMPS; /* XXX is this necessary? dmq */
			    bufflen = TRIE_INITAL_ACCEPT_BUFFLEN;
			    sv_accept_buff=newSV(bufflen *
					    sizeof(reg_trie_accepted) - 1);
			    SvCUR_set(sv_accept_buff, 0);
			    SvPOK_on(sv_accept_buff);
			    sv_2mortal(sv_accept_buff);
			    SAVETMPS;
			    ST.accept_buff =
				(reg_trie_accepted*)SvPV_nolen(sv_accept_buff );
			}
			do {
			    if (ST.accepted >= bufflen) {
				bufflen *= 2;
				ST.accept_buff =(reg_trie_accepted*)
				    SvGROW(sv_accept_buff,
				       	bufflen * sizeof(reg_trie_accepted));
			    }
			    SvCUR_set(sv_accept_buff,SvCUR(sv_accept_buff)
				+ sizeof(reg_trie_accepted));


			    ST.accept_buff[ST.accepted].wordnum = got_wordnum;
			    ST.accept_buff[ST.accepted].endpos = uc;
			    ++ST.accepted;
		        } while (trie->nextword && (got_wordnum= trie->nextword[got_wordnum]));
		    }
#undef got_wordnum 

		    DEBUG_TRIE_EXECUTE_r({
		                DUMP_EXEC_POS( (char *)uc, scan, do_utf8 );
			        PerlIO_printf( Perl_debug_log,
			            "%*s  %sState: %4"UVxf" Accepted: %4"UVxf" ",
			            2+depth * 2, "", PL_colors[4],
			            (UV)state, (UV)ST.accepted );
		    });

		    if ( base ) {
			REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc,
					     uscan, len, uvc, charid, foldlen,
					     foldbuf, uniflags);

			if (charid &&
			     (base + charid > trie->uniquecharcount )
			     && (base + charid - 1 - trie->uniquecharcount
				    < trie->lasttrans)
			     && trie->trans[base + charid - 1 -
				    trie->uniquecharcount].check == state)
			{
			    state = trie->trans[base + charid - 1 -
				trie->uniquecharcount ].next;
			}
			else {
			    state = 0;
			}
			uc += len;

		    }
		    else {
			state = 0;
		    }
		    DEBUG_TRIE_EXECUTE_r(
		        PerlIO_printf( Perl_debug_log,
		            "Charid:%3x CP:%4"UVxf" After State: %4"UVxf"%s\n",
		            charid, uvc, (UV)state, PL_colors[5] );
		    );
		}
		if (!ST.accepted )
		   sayNO;

		DEBUG_EXECUTE_r(
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sgot %"IVdf" possible matches%s\n",
			REPORT_CODE_OFF + depth * 2, "",
			PL_colors[4], (IV)ST.accepted, PL_colors[5] );
		);
	    }}
            goto trie_first_try; /* jump into the fail handler */
	    /* NOTREACHED */
	case TRIE_next_fail: /* we failed - try next alterative */
            if ( ST.jump) {
                REGCP_UNWIND(ST.cp);
	        for (n = *PL_reglastparen; n > ST.lastparen; n--)
		    PL_regoffs[n].end = -1;
	        *PL_reglastparen = n;
	    }
          trie_first_try:
            if (do_cutgroup) {
                do_cutgroup = 0;
                no_final = 0;
            }

            if ( ST.jump) {
                ST.lastparen = *PL_reglastparen;
	        REGCP_SET(ST.cp);
            }	        
	    if ( ST.accepted == 1 ) {
		/* only one choice left - just continue */
		DEBUG_EXECUTE_r({
		    AV *const trie_words
			= MUTABLE_AV(rexi->data->data[ARG(ST.me)+TRIE_WORDS_OFFSET]);
		    SV ** const tmp = av_fetch( trie_words, 
		        ST.accept_buff[ 0 ].wordnum-1, 0 );
		    SV *sv= tmp ? sv_newmortal() : NULL;
		    
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sonly one match left: #%d <%s>%s\n",
			REPORT_CODE_OFF+depth*2, "", PL_colors[4],
			ST.accept_buff[ 0 ].wordnum,
			tmp ? pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 0, 
	                        PL_colors[0], PL_colors[1],
	                        (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0)
                            ) 
			: "not compiled under -Dr",
			PL_colors[5] );
		});
		PL_reginput = (char *)ST.accept_buff[ 0 ].endpos;
		/* in this case we free tmps/leave before we call regmatch
		   as we wont be using accept_buff again. */
		
		locinput = PL_reginput;
		nextchr = UCHARAT(locinput);
    		if ( !ST.jump || !ST.jump[ST.accept_buff[0].wordnum]) 
    		    scan = ST.B;
    		else
    		    scan = ST.me + ST.jump[ST.accept_buff[0].wordnum];
		if (!has_cutgroup) {
		    FREETMPS;
		    LEAVE;
                } else {
                    ST.accepted--;
                    PUSH_YES_STATE_GOTO(TRIE_next, scan);
                }
		
		continue; /* execute rest of RE */
	    }
	    
	    if ( !ST.accepted-- ) {
	        DEBUG_EXECUTE_r({
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sTRIE failed...%s\n",
			REPORT_CODE_OFF+depth*2, "", 
			PL_colors[4],
			PL_colors[5] );
		});
		FREETMPS;
		LEAVE;
		sayNO_SILENT;
		/*NOTREACHED*/
	    } 

	    /*
	       There are at least two accepting states left.  Presumably
	       the number of accepting states is going to be low,
	       typically two. So we simply scan through to find the one
	       with lowest wordnum.  Once we find it, we swap the last
	       state into its place and decrement the size. We then try to
	       match the rest of the pattern at the point where the word
	       ends. If we succeed, control just continues along the
	       regex; if we fail we return here to try the next accepting
	       state
	     */

	    {
		U32 best = 0;
		U32 cur;
		for( cur = 1 ; cur <= ST.accepted ; cur++ ) {
		    DEBUG_TRIE_EXECUTE_r(
			PerlIO_printf( Perl_debug_log,
			    "%*s  %sgot %"IVdf" (%d) as best, looking at %"IVdf" (%d)%s\n",
			    REPORT_CODE_OFF + depth * 2, "", PL_colors[4],
			    (IV)best, ST.accept_buff[ best ].wordnum, (IV)cur,
			    ST.accept_buff[ cur ].wordnum, PL_colors[5] );
		    );

		    if (ST.accept_buff[cur].wordnum <
			    ST.accept_buff[best].wordnum)
			best = cur;
		}

		DEBUG_EXECUTE_r({
		    AV *const trie_words
			= MUTABLE_AV(rexi->data->data[ARG(ST.me)+TRIE_WORDS_OFFSET]);
		    SV ** const tmp = av_fetch( trie_words, 
		        ST.accept_buff[ best ].wordnum - 1, 0 );
		    regnode *nextop=(!ST.jump || !ST.jump[ST.accept_buff[best].wordnum]) ? 
		                    ST.B : 
		                    ST.me + ST.jump[ST.accept_buff[best].wordnum];    
		    SV *sv= tmp ? sv_newmortal() : NULL;
		    
		    PerlIO_printf( Perl_debug_log, 
		        "%*s  %strying alternation #%d <%s> at node #%d %s\n",
			REPORT_CODE_OFF+depth*2, "", PL_colors[4],
			ST.accept_buff[best].wordnum,
			tmp ? pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 0, 
	                        PL_colors[0], PL_colors[1],
	                        (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0)
                            ) : "not compiled under -Dr", 
			    REG_NODE_NUM(nextop),
			PL_colors[5] );
		});

		if ( best<ST.accepted ) {
		    reg_trie_accepted tmp = ST.accept_buff[ best ];
		    ST.accept_buff[ best ] = ST.accept_buff[ ST.accepted ];
		    ST.accept_buff[ ST.accepted ] = tmp;
		    best = ST.accepted;
		}
		PL_reginput = (char *)ST.accept_buff[ best ].endpos;
		if ( !ST.jump || !ST.jump[ST.accept_buff[best].wordnum]) {
		    scan = ST.B;
		} else {
		    scan = ST.me + ST.jump[ST.accept_buff[best].wordnum];
                }
                PUSH_YES_STATE_GOTO(TRIE_next, scan);    
                /* NOTREACHED */
	    }
	    /* NOTREACHED */
        case TRIE_next:
	    /* we dont want to throw this away, see bug 57042*/
	    if (oreplsv != GvSV(PL_replgv))
		sv_setsv(oreplsv, GvSV(PL_replgv));
            FREETMPS;
	    LEAVE;
	    sayYES;
#undef  ST

	case EXACT: {
	    char *s = STRING(scan);
	    ln = STR_LEN(scan);
	    if (do_utf8 != UTF) {
		/* The target and the pattern have differing utf8ness. */
		char *l = locinput;
		const char * const e = s + ln;

		if (do_utf8) {
		    /* The target is utf8, the pattern is not utf8. */
		    while (s < e) {
			STRLEN ulen;
			if (l >= PL_regeol)
			     sayNO;
			if (NATIVE_TO_UNI(*(U8*)s) !=
			    utf8n_to_uvuni((U8*)l, UTF8_MAXBYTES, &ulen,
					    uniflags))
			     sayNO;
			l += ulen;
			s ++;
		    }
		}
		else {
		    /* The target is not utf8, the pattern is utf8. */
		    while (s < e) {
			STRLEN ulen;
			if (l >= PL_regeol)
			    sayNO;
			if (NATIVE_TO_UNI(*((U8*)l)) !=
			    utf8n_to_uvuni((U8*)s, UTF8_MAXBYTES, &ulen,
					   uniflags))
			    sayNO;
			s += ulen;
			l ++;
		    }
		}
		locinput = l;
		nextchr = UCHARAT(locinput);
		break;
	    }
	    /* The target and the pattern have the same utf8ness. */
	    /* Inline the first character, for speed. */
	    if (UCHARAT(s) != nextchr)
		sayNO;
	    if (PL_regeol - locinput < ln)
		sayNO;
	    if (ln > 1 && memNE(s, locinput, ln))
		sayNO;
	    locinput += ln;
	    nextchr = UCHARAT(locinput);
	    break;
	    }
	case EXACTFL:
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
	case EXACTF: {
	    char * const s = STRING(scan);
	    ln = STR_LEN(scan);

	    if (do_utf8 || UTF) {
	      /* Either target or the pattern are utf8. */
		const char * const l = locinput;
		char *e = PL_regeol;

		if (ibcmp_utf8(s, 0,  ln, (bool)UTF,
			       l, &e, 0,  do_utf8)) {
		     /* One more case for the sharp s:
		      * pack("U0U*", 0xDF) =~ /ss/i,
		      * the 0xC3 0x9F are the UTF-8
		      * byte sequence for the U+00DF. */

		     if (!(do_utf8 &&
		           toLOWER(s[0]) == 's' &&
			   ln >= 2 &&
			   toLOWER(s[1]) == 's' &&
			   (U8)l[0] == 0xC3 &&
			   e - l >= 2 &&
			   (U8)l[1] == 0x9F))
			  sayNO;
		}
		locinput = e;
		nextchr = UCHARAT(locinput);
		break;
	    }

	    /* Neither the target and the pattern are utf8. */

	    /* Inline the first character, for speed. */
	    if (UCHARAT(s) != nextchr &&
		UCHARAT(s) != ((OP(scan) == EXACTF)
			       ? PL_fold : PL_fold_locale)[nextchr])
		sayNO;
	    if (PL_regeol - locinput < ln)
		sayNO;
	    if (ln > 1 && (OP(scan) == EXACTF
			   ? ibcmp(s, locinput, ln)
			   : ibcmp_locale(s, locinput, ln)))
		sayNO;
	    locinput += ln;
	    nextchr = UCHARAT(locinput);
	    break;
	    }
	case BOUNDL:
	case NBOUNDL:
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
	case BOUND:
	case NBOUND:
	    /* was last char in word? */
	    if (do_utf8) {
		if (locinput == PL_bostr)
		    ln = '\n';
		else {
		    const U8 * const r = reghop3((U8*)locinput, -1, (U8*)PL_bostr);

		    ln = utf8n_to_uvchr(r, UTF8SKIP(r), 0, uniflags);
		}
		if (OP(scan) == BOUND || OP(scan) == NBOUND) {
		    ln = isALNUM_uni(ln);
		    LOAD_UTF8_CHARCLASS_ALNUM();
		    n = swash_fetch(PL_utf8_alnum, (U8*)locinput, do_utf8);
		}
		else {
		    ln = isALNUM_LC_uvchr(UNI_TO_NATIVE(ln));
		    n = isALNUM_LC_utf8((U8*)locinput);
		}
	    }
	    else {
		ln = (locinput != PL_bostr) ?
		    UCHARAT(locinput - 1) : '\n';
		if (OP(scan) == BOUND || OP(scan) == NBOUND) {
		    ln = isALNUM(ln);
		    n = isALNUM(nextchr);
		}
		else {
		    ln = isALNUM_LC(ln);
		    n = isALNUM_LC(nextchr);
		}
	    }
	    if (((!ln) == (!n)) == (OP(scan) == BOUND ||
				    OP(scan) == BOUNDL))
		    sayNO;
	    break;
	case ANYOF:
	    if (do_utf8) {
	        STRLEN inclasslen = PL_regeol - locinput;

	        if (!reginclass(rex, scan, (U8*)locinput, &inclasslen, do_utf8))
		    goto anyof_fail;
		if (locinput >= PL_regeol)
		    sayNO;
		locinput += inclasslen ? inclasslen : UTF8SKIP(locinput);
		nextchr = UCHARAT(locinput);
		break;
	    }
	    else {
		if (nextchr < 0)
		    nextchr = UCHARAT(locinput);
		if (!REGINCLASS(rex, scan, (U8*)locinput))
		    goto anyof_fail;
		if (!nextchr && locinput >= PL_regeol)
		    sayNO;
		nextchr = UCHARAT(++locinput);
		break;
	    }
	anyof_fail:
	    /* If we might have the case of the German sharp s
	     * in a casefolding Unicode character class. */

	    if (ANYOF_FOLD_SHARP_S(scan, locinput, PL_regeol)) {
		 locinput += SHARP_S_SKIP;
		 nextchr = UCHARAT(locinput);
	    }
	    else
		 sayNO;
	    break;
	/* Special char classes - The defines start on line 129 or so */
	CCC_TRY_AFF( ALNUM,  ALNUML, perl_word,   "a", isALNUM_LC_utf8, isALNUM, isALNUM_LC);
	CCC_TRY_NEG(NALNUM, NALNUML, perl_word,   "a", isALNUM_LC_utf8, isALNUM, isALNUM_LC);

	CCC_TRY_AFF( SPACE,  SPACEL, perl_space,  " ", isSPACE_LC_utf8, isSPACE, isSPACE_LC);
	CCC_TRY_NEG(NSPACE, NSPACEL, perl_space,  " ", isSPACE_LC_utf8, isSPACE, isSPACE_LC);

	CCC_TRY_AFF( DIGIT,  DIGITL, posix_digit, "0", isDIGIT_LC_utf8, isDIGIT, isDIGIT_LC);
	CCC_TRY_NEG(NDIGIT, NDIGITL, posix_digit, "0", isDIGIT_LC_utf8, isDIGIT, isDIGIT_LC);

	case CLUMP: /* Match \X: logical Unicode character.  This is defined as
		       a Unicode extended Grapheme Cluster */
	    /* From http://www.unicode.org/reports/tr29 (5.2 version).  An
	      extended Grapheme Cluster is:

	       CR LF
	       | Prepend* Begin Extend*
	       | .

	       Begin is (Hangul-syllable | ! Control)
	       Extend is (Grapheme_Extend | Spacing_Mark)
	       Control is [ GCB_Control CR LF ]

	       The discussion below shows how the code for CLUMP is derived
	       from this regex.  Note that most of these concepts are from
	       property values of the Grapheme Cluster Boundary (GCB) property.
	       No code point can have multiple property values for a given
	       property.  Thus a code point in Prepend can't be in Control, but
	       it must be in !Control.  This is why Control above includes
	       GCB_Control plus CR plus LF.  The latter two are used in the GCB
	       property separately, and so can't be in GCB_Control, even though
	       they logically are controls.  Control is not the same as gc=cc,
	       but includes format and other characters as well.

	       The Unicode definition of Hangul-syllable is:
		   L+
		   | (L* ( ( V | LV ) V* | LVT ) T*)
		   | T+ 
		  )
	       Each of these is a value for the GCB property, and hence must be
	       disjoint, so the order they are tested is immaterial, so the
	       above can safely be changed to
		   T+
		   | L+
		   | (L* ( LVT | ( V | LV ) V*) T*)

	       The last two terms can be combined like this:
		   L* ( L
		        | (( LVT | ( V | LV ) V*) T*))

	       And refactored into this:
		   L* (L | LVT T* | V  V* T* | LV  V* T*)

	       That means that if we have seen any L's at all we can quit
	       there, but if the next character is a LVT, a V or and LV we
	       should keep going.

	       There is a subtlety with Prepend* which showed up in testing.
	       Note that the Begin, and only the Begin is required in:
	        | Prepend* Begin Extend*
	       Also, Begin contains '! Control'.  A Prepend must be a '!
	       Control', which means it must be a Begin.  What it comes down to
	       is that if we match Prepend* and then find no suitable Begin
	       afterwards, that if we backtrack the last Prepend, that one will
	       be a suitable Begin.
	    */

	    if (locinput >= PL_regeol)
		sayNO;
	    if  (! do_utf8) {

		/* Match either CR LF  or '.', as all the other possibilities
		 * require utf8 */
		locinput++;	    /* Match the . or CR */
		if (nextchr == '\r'
		    && locinput < PL_regeol
		    && UCHARAT(locinput) == '\n') locinput++;
	    }
	    else {

		/* Utf8: See if is ( CR LF ); already know that locinput <
		 * PL_regeol, so locinput+1 is in bounds */
		if (nextchr == '\r' && UCHARAT(locinput + 1) == '\n') {
		    locinput += 2;
		}
		else {
		    /* In case have to backtrack to beginning, then match '.' */
		    char *starting = locinput;

		    /* In case have to backtrack the last prepend */
		    char *previous_prepend = 0;

		    LOAD_UTF8_CHARCLASS_GCB();

		    /* Match (prepend)* */
		    while (locinput < PL_regeol
			   && swash_fetch(PL_utf8_X_prepend,
					  (U8*)locinput, do_utf8))
		    {
			previous_prepend = locinput;
			locinput += UTF8SKIP(locinput);
		    }

		    /* As noted above, if we matched a prepend character, but
		     * the next thing won't match, back off the last prepend we
		     * matched, as it is guaranteed to match the begin */
		    if (previous_prepend
			&& (locinput >=  PL_regeol
			    || ! swash_fetch(PL_utf8_X_begin,
					     (U8*)locinput, do_utf8)))
		    {
			locinput = previous_prepend;
		    }

		    /* Note that here we know PL_regeol > locinput, as we
		     * tested that upon input to this switch case, and if we
		     * moved locinput forward, we tested the result just above
		     * and it either passed, or we backed off so that it will
		     * now pass */
		    if (! swash_fetch(PL_utf8_X_begin, (U8*)locinput, do_utf8)) {

			/* Here did not match the required 'Begin' in the
			 * second term.  So just match the very first
			 * character, the '.' of the final term of the regex */
			locinput = starting + UTF8SKIP(starting);
		    } else {

			/* Here is the beginning of a character that can have
			 * an extender.  It is either a hangul syllable, or a
			 * non-control */
			if (swash_fetch(PL_utf8_X_non_hangul,
					(U8*)locinput, do_utf8))
			{

			    /* Here not a Hangul syllable, must be a
			     * ('!  * Control') */
			    locinput += UTF8SKIP(locinput);
			} else {

			    /* Here is a Hangul syllable.  It can be composed
			     * of several individual characters.  One
			     * possibility is T+ */
			    if (swash_fetch(PL_utf8_X_T,
					    (U8*)locinput, do_utf8))
			    {
				while (locinput < PL_regeol
					&& swash_fetch(PL_utf8_X_T,
							(U8*)locinput, do_utf8))
				{
				    locinput += UTF8SKIP(locinput);
				}
			    } else {

				/* Here, not T+, but is a Hangul.  That means
				 * it is one of the others: L, LV, LVT or V,
				 * and matches:
				 * L* (L | LVT T* | V  V* T* | LV  V* T*) */

				/* Match L*           */
				while (locinput < PL_regeol
					&& swash_fetch(PL_utf8_X_L,
							(U8*)locinput, do_utf8))
				{
				    locinput += UTF8SKIP(locinput);
				}

				/* Here, have exhausted L*.  If the next
				 * character is not an LV, LVT nor V, it means
				 * we had to have at least one L, so matches L+
				 * in the original equation, we have a complete
				 * hangul syllable.  Are done. */

				if (locinput < PL_regeol
				    && swash_fetch(PL_utf8_X_LV_LVT_V,
						    (U8*)locinput, do_utf8))
				{

				    /* Otherwise keep going.  Must be LV, LVT
				     * or V.  See if LVT */
				    if (swash_fetch(PL_utf8_X_LVT,
						    (U8*)locinput, do_utf8))
				    {
					locinput += UTF8SKIP(locinput);
				    } else {

					/* Must be  V or LV.  Take it, then
					 * match V*     */
					locinput += UTF8SKIP(locinput);
					while (locinput < PL_regeol
						&& swash_fetch(PL_utf8_X_V,
							 (U8*)locinput, do_utf8))
					{
					    locinput += UTF8SKIP(locinput);
					}
				    }

				    /* And any of LV, LVT, or V can be followed
				     * by T*            */
				    while (locinput < PL_regeol
					   && swash_fetch(PL_utf8_X_T,
							   (U8*)locinput,
							   do_utf8))
				    {
					locinput += UTF8SKIP(locinput);
				    }
				}
			    }
			}

			/* Match any extender */
			while (locinput < PL_regeol
				&& swash_fetch(PL_utf8_X_extend,
						(U8*)locinput, do_utf8))
			{
			    locinput += UTF8SKIP(locinput);
			}
		    }
		}
		if (locinput > PL_regeol) sayNO;
	    }
	    nextchr = UCHARAT(locinput);
	    break;
            
	case NREFFL:
	{
	    char *s;
	    char type;
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
	case NREF:
	case NREFF:
	    type = OP(scan);
	    n = reg_check_named_buff_matched(rex,scan);

            if ( n ) {
                type = REF + ( type - NREF );
                goto do_ref;
            } else {
                sayNO;
            }
            /* unreached */
	case REFFL:
	    PL_reg_flags |= RF_tainted;
	    /* FALL THROUGH */
        case REF:
	case REFF: 
	    n = ARG(scan);  /* which paren pair */
	    type = OP(scan);
	  do_ref:  
	    ln = PL_regoffs[n].start;
	    PL_reg_leftiter = PL_reg_maxiter;		/* Void cache */
	    if (*PL_reglastparen < n || ln == -1)
		sayNO;			/* Do not match unless seen CLOSEn. */
	    if (ln == PL_regoffs[n].end)
		break;

	    s = PL_bostr + ln;
	    if (do_utf8 && type != REF) {	/* REF can do byte comparison */
		char *l = locinput;
		const char *e = PL_bostr + PL_regoffs[n].end;
		/*
		 * Note that we can't do the "other character" lookup trick as
		 * in the 8-bit case (no pun intended) because in Unicode we
		 * have to map both upper and title case to lower case.
		 */
		if (type == REFF) {
		    while (s < e) {
			STRLEN ulen1, ulen2;
			U8 tmpbuf1[UTF8_MAXBYTES_CASE+1];
			U8 tmpbuf2[UTF8_MAXBYTES_CASE+1];

			if (l >= PL_regeol)
			    sayNO;
			toLOWER_utf8((U8*)s, tmpbuf1, &ulen1);
			toLOWER_utf8((U8*)l, tmpbuf2, &ulen2);
			if (ulen1 != ulen2 || memNE((char *)tmpbuf1, (char *)tmpbuf2, ulen1))
			    sayNO;
			s += ulen1;
			l += ulen2;
		    }
		}
		locinput = l;
		nextchr = UCHARAT(locinput);
		break;
	    }

	    /* Inline the first character, for speed. */
	    if (UCHARAT(s) != nextchr &&
		(type == REF ||
		 (UCHARAT(s) != (type == REFF
				  ? PL_fold : PL_fold_locale)[nextchr])))
		sayNO;
	    ln = PL_regoffs[n].end - ln;
	    if (locinput + ln > PL_regeol)
		sayNO;
	    if (ln > 1 && (type == REF
			   ? memNE(s, locinput, ln)
			   : (type == REFF
			      ? ibcmp(s, locinput, ln)
			      : ibcmp_locale(s, locinput, ln))))
		sayNO;
	    locinput += ln;
	    nextchr = UCHARAT(locinput);
	    break;
	}
	case NOTHING:
	case TAIL:
	    break;
	case BACK:
	    break;

#undef  ST
#define ST st->u.eval
	{
	    SV *ret;
	    REGEXP *re_sv;
            regexp *re;
            regexp_internal *rei;
            regnode *startpoint;

	case GOSTART:
	case GOSUB: /*    /(...(?1))/   /(...(?&foo))/   */
	    if (cur_eval && cur_eval->locinput==locinput) {
                if (cur_eval->u.eval.close_paren == (U32)ARG(scan)) 
                    Perl_croak(aTHX_ "Infinite recursion in regex");
                if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_ 
                        "Pattern subroutine nesting without pos change"
                        " exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }
	    re_sv = rex_sv;
            re = rex;
            rei = rexi;
            (void)ReREFCNT_inc(rex_sv);
            if (OP(scan)==GOSUB) {
                startpoint = scan + ARG2L(scan);
                ST.close_paren = ARG(scan);
            } else {
                startpoint = rei->program+1;
                ST.close_paren = 0;
            }
            goto eval_recurse_doit;
            /* NOTREACHED */
        case EVAL:  /*   /(?{A})B/   /(??{A})B/  and /(?(?{A})X|Y)B/   */        
            if (cur_eval && cur_eval->locinput==locinput) {
		if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_ "EVAL without pos change exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }    
	    {
		/* execute the code in the {...} */
		dSP;
		SV ** const before = SP;
		OP_4tree * const oop = PL_op;
		COP * const ocurcop = PL_curcop;
		PAD *old_comppad;
		char *saved_regeol = PL_regeol;
	    
		n = ARG(scan);
		PL_op = (OP_4tree*)rexi->data->data[n];
		DEBUG_STATE_r( PerlIO_printf(Perl_debug_log, 
		    "  re_eval 0x%"UVxf"\n", PTR2UV(PL_op)) );
		PAD_SAVE_LOCAL(old_comppad, (PAD*)rexi->data->data[n + 2]);
		PL_regoffs[0].end = PL_reg_magic->mg_len = locinput - PL_bostr;

                if (sv_yes_mark) {
                    SV *sv_mrk = get_sv("REGMARK", 1);
                    sv_setsv(sv_mrk, sv_yes_mark);
                }

		CALLRUNOPS(aTHX);			/* Scalar context. */
		SPAGAIN;
		if (SP == before)
		    ret = &PL_sv_undef;   /* protect against empty (?{}) blocks. */
		else {
		    ret = POPs;
		    PUTBACK;
		}

		PL_op = oop;
		PAD_RESTORE_LOCAL(old_comppad);
		PL_curcop = ocurcop;
		PL_regeol = saved_regeol;
		if (!logical) {
		    /* /(?{...})/ */
		    sv_setsv(save_scalar(PL_replgv), ret);
		    break;
		}
	    }
	    if (logical == 2) { /* Postponed subexpression: /(??{...})/ */
		logical = 0;
		{
		    /* extract RE object from returned value; compiling if
		     * necessary */
		    MAGIC *mg = NULL;
		    REGEXP *rx = NULL;

		    if (SvROK(ret)) {
			SV *const sv = SvRV(ret);

			if (SvTYPE(sv) == SVt_REGEXP) {
			    rx = (REGEXP*) sv;
			} else if (SvSMAGICAL(sv)) {
			    mg = mg_find(sv, PERL_MAGIC_qr);
			    assert(mg);
			}
		    } else if (SvTYPE(ret) == SVt_REGEXP) {
			rx = (REGEXP*) ret;
		    } else if (SvSMAGICAL(ret)) {
			if (SvGMAGICAL(ret)) {
			    /* I don't believe that there is ever qr magic
			       here.  */
			    assert(!mg_find(ret, PERL_MAGIC_qr));
			    sv_unmagic(ret, PERL_MAGIC_qr);
			}
			else {
			    mg = mg_find(ret, PERL_MAGIC_qr);
			    /* testing suggests mg only ends up non-NULL for
			       scalars who were upgraded and compiled in the
			       else block below. In turn, this is only
			       triggered in the "postponed utf8 string" tests
			       in t/op/pat.t  */
			}
		    }

		    if (mg) {
			rx = (REGEXP *) mg->mg_obj; /*XXX:dmq*/
			assert(rx);
		    }
		    if (rx) {
			rx = reg_temp_copy(NULL, rx);
		    }
		    else {
			U32 pm_flags = 0;
			const I32 osize = PL_regsize;

			if (DO_UTF8(ret)) {
			    assert (SvUTF8(ret));
			} else if (SvUTF8(ret)) {
			    /* Not doing UTF-8, despite what the SV says. Is
			       this only if we're trapped in use 'bytes'?  */
			    /* Make a copy of the octet sequence, but without
			       the flag on, as the compiler now honours the
			       SvUTF8 flag on ret.  */
			    STRLEN len;
			    const char *const p = SvPV(ret, len);
			    ret = newSVpvn_flags(p, len, SVs_TEMP);
			}
			rx = CALLREGCOMP(ret, pm_flags);
			if (!(SvFLAGS(ret)
			      & (SVs_TEMP | SVs_PADTMP | SVf_READONLY
				 | SVs_GMG))) {
			    /* This isn't a first class regexp. Instead, it's
			       caching a regexp onto an existing, Perl visible
			       scalar.  */
			    sv_magic(ret, MUTABLE_SV(rx), PERL_MAGIC_qr, 0, 0);
			}
			PL_regsize = osize;
		    }
		    re_sv = rx;
		    re = (struct regexp *)SvANY(rx);
		}
                RXp_MATCH_COPIED_off(re);
                re->subbeg = rex->subbeg;
                re->sublen = rex->sublen;
		rei = RXi_GET(re);
                DEBUG_EXECUTE_r(
                    debug_start_match(re_sv, do_utf8, locinput, PL_regeol, 
                        "Matching embedded");
		);		
		startpoint = rei->program + 1;
               	ST.close_paren = 0; /* only used for GOSUB */
               	/* borrowed from regtry */
                if (PL_reg_start_tmpl <= re->nparens) {
                    PL_reg_start_tmpl = re->nparens*3/2 + 3;
                    if(PL_reg_start_tmp)
                        Renew(PL_reg_start_tmp, PL_reg_start_tmpl, char*);
                    else
                        Newx(PL_reg_start_tmp, PL_reg_start_tmpl, char*);
                }               	

        eval_recurse_doit: /* Share code with GOSUB below this line */                		
		/* run the pattern returned from (??{...}) */
		ST.cp = regcppush(0);	/* Save *all* the positions. */
		REGCP_SET(ST.lastcp);
		
		PL_regoffs = re->offs; /* essentially NOOP on GOSUB */
		
		/* see regtry, specifically PL_reglast(?:close)?paren is a pointer! (i dont know why) :dmq */
		PL_reglastparen = &re->lastparen;
		PL_reglastcloseparen = &re->lastcloseparen;
		re->lastparen = 0;
		re->lastcloseparen = 0;

		PL_reginput = locinput;
		PL_regsize = 0;

		/* XXXX This is too dramatic a measure... */
		PL_reg_maxiter = 0;

		ST.toggle_reg_flags = PL_reg_flags;
		if (RX_UTF8(re_sv))
		    PL_reg_flags |= RF_utf8;
		else
		    PL_reg_flags &= ~RF_utf8;
		ST.toggle_reg_flags ^= PL_reg_flags; /* diff of old and new */

		ST.prev_rex = rex_sv;
		ST.prev_curlyx = cur_curlyx;
		SETREX(rex_sv,re_sv);
		rex = re;
		rexi = rei;
		cur_curlyx = NULL;
		ST.B = next;
		ST.prev_eval = cur_eval;
		cur_eval = st;
		/* now continue from first node in postoned RE */
		PUSH_YES_STATE_GOTO(EVAL_AB, startpoint);
		/* NOTREACHED */
	    }
	    /* logical is 1,   /(?(?{...})X|Y)/ */
	    sw = (bool)SvTRUE(ret);
	    logical = 0;
	    break;
	}

	case EVAL_AB: /* cleanup after a successful (??{A})B */
	    /* note: this is called twice; first after popping B, then A */
	    PL_reg_flags ^= ST.toggle_reg_flags; 
	    ReREFCNT_dec(rex_sv);
	    SETREX(rex_sv,ST.prev_rex);
	    rex = (struct regexp *)SvANY(rex_sv);
	    rexi = RXi_GET(rex);
	    regcpblow(ST.cp);
	    cur_eval = ST.prev_eval;
	    cur_curlyx = ST.prev_curlyx;

	    /* rex was changed so update the pointer in PL_reglastparen and PL_reglastcloseparen */
	    PL_reglastparen = &rex->lastparen;
	    PL_reglastcloseparen = &rex->lastcloseparen;
	    /* also update PL_regoffs */
	    PL_regoffs = rex->offs;
	    
	    /* XXXX This is too dramatic a measure... */
	    PL_reg_maxiter = 0;
            if ( nochange_depth )
	        nochange_depth--;
	    sayYES;


	case EVAL_AB_fail: /* unsuccessfully ran A or B in (??{A})B */
	    /* note: this is called twice; first after popping B, then A */
	    PL_reg_flags ^= ST.toggle_reg_flags; 
	    ReREFCNT_dec(rex_sv);
	    SETREX(rex_sv,ST.prev_rex);
	    rex = (struct regexp *)SvANY(rex_sv);
	    rexi = RXi_GET(rex); 
	    /* rex was changed so update the pointer in PL_reglastparen and PL_reglastcloseparen */
	    PL_reglastparen = &rex->lastparen;
	    PL_reglastcloseparen = &rex->lastcloseparen;

	    PL_reginput = locinput;
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex);
	    cur_eval = ST.prev_eval;
	    cur_curlyx = ST.prev_curlyx;
	    /* XXXX This is too dramatic a measure... */
	    PL_reg_maxiter = 0;
	    if ( nochange_depth )
	        nochange_depth--;
	    sayNO_SILENT;
#undef ST

	case OPEN:
	    n = ARG(scan);  /* which paren pair */
	    PL_reg_start_tmp[n] = locinput;
	    if (n > PL_regsize)
		PL_regsize = n;
            lastopen = n;
	    break;
	case CLOSE:
	    n = ARG(scan);  /* which paren pair */
	    PL_regoffs[n].start = PL_reg_start_tmp[n] - PL_bostr;
	    PL_regoffs[n].end = locinput - PL_bostr;
	    /*if (n > PL_regsize)
		PL_regsize = n;*/
	    if (n > *PL_reglastparen)
		*PL_reglastparen = n;
	    *PL_reglastcloseparen = n;
            if (cur_eval && cur_eval->u.eval.close_paren == n) {
	        goto fake_end;
	    }    
	    break;
        case ACCEPT:
            if (ARG(scan)){
                regnode *cursor;
                for (cursor=scan;
                     cursor && OP(cursor)!=END; 
                     cursor=regnext(cursor)) 
                {
                    if ( OP(cursor)==CLOSE ){
                        n = ARG(cursor);
                        if ( n <= lastopen ) {
                            PL_regoffs[n].start
				= PL_reg_start_tmp[n] - PL_bostr;
                            PL_regoffs[n].end = locinput - PL_bostr;
                            /*if (n > PL_regsize)
                            PL_regsize = n;*/
                            if (n > *PL_reglastparen)
                                *PL_reglastparen = n;
                            *PL_reglastcloseparen = n;
                            if ( n == ARG(scan) || (cur_eval &&
                                cur_eval->u.eval.close_paren == n))
                                break;
                        }
                    }
                }
            }
	    goto fake_end;
	    /*NOTREACHED*/	    
	case GROUPP:
	    n = ARG(scan);  /* which paren pair */
	    sw = (bool)(*PL_reglastparen >= n && PL_regoffs[n].end != -1);
	    break;
	case NGROUPP:
	    /* reg_check_named_buff_matched returns 0 for no match */
	    sw = (bool)(0 < reg_check_named_buff_matched(rex,scan));
	    break;
        case INSUBP:
            n = ARG(scan);
            sw = (cur_eval && (!n || cur_eval->u.eval.close_paren == n));
            break;
        case DEFINEP:
            sw = 0;
            break;
	case IFTHEN:
	    PL_reg_leftiter = PL_reg_maxiter;		/* Void cache */
	    if (sw)
		next = NEXTOPER(NEXTOPER(scan));
	    else {
		next = scan + ARG(scan);
		if (OP(next) == IFTHEN) /* Fake one. */
		    next = NEXTOPER(NEXTOPER(next));
	    }
	    break;
	case LOGICAL:
	    logical = scan->flags;
	    break;

/*******************************************************************

The CURLYX/WHILEM pair of ops handle the most generic case of the /A*B/
pattern, where A and B are subpatterns. (For simple A, CURLYM or
STAR/PLUS/CURLY/CURLYN are used instead.)

A*B is compiled as <CURLYX><A><WHILEM><B>

On entry to the subpattern, CURLYX is called. This pushes a CURLYX
state, which contains the current count, initialised to -1. It also sets
cur_curlyx to point to this state, with any previous value saved in the
state block.

CURLYX then jumps straight to the WHILEM op, rather than executing A,
since the pattern may possibly match zero times (i.e. it's a while {} loop
rather than a do {} while loop).

Each entry to WHILEM represents a successful match of A. The count in the
CURLYX block is incremented, another WHILEM state is pushed, and execution
passes to A or B depending on greediness and the current count.

For example, if matching against the string a1a2a3b (where the aN are
substrings that match /A/), then the match progresses as follows: (the
pushed states are interspersed with the bits of strings matched so far):

    <CURLYX cnt=-1>
    <CURLYX cnt=0><WHILEM>
    <CURLYX cnt=1><WHILEM> a1 <WHILEM>
    <CURLYX cnt=2><WHILEM> a1 <WHILEM> a2 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM> b

(Contrast this with something like CURLYM, which maintains only a single
backtrack state:

    <CURLYM cnt=0> a1
    a1 <CURLYM cnt=1> a2
    a1 a2 <CURLYM cnt=2> a3
    a1 a2 a3 <CURLYM cnt=3> b
)

Each WHILEM state block marks a point to backtrack to upon partial failure
of A or B, and also contains some minor state data related to that
iteration.  The CURLYX block, pointed to by cur_curlyx, contains the
overall state, such as the count, and pointers to the A and B ops.

This is complicated slightly by nested CURLYX/WHILEM's. Since cur_curlyx
must always point to the *current* CURLYX block, the rules are:

When executing CURLYX, save the old cur_curlyx in the CURLYX state block,
and set cur_curlyx to point the new block.

When popping the CURLYX block after a successful or unsuccessful match,
restore the previous cur_curlyx.

When WHILEM is about to execute B, save the current cur_curlyx, and set it
to the outer one saved in the CURLYX block.

When popping the WHILEM block after a successful or unsuccessful B match,
restore the previous cur_curlyx.

Here's an example for the pattern (AI* BI)*BO
I and O refer to inner and outer, C and W refer to CURLYX and WHILEM:

cur_
curlyx backtrack stack
------ ---------------
NULL   
CO     <CO prev=NULL> <WO>
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai 
CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi 
NULL   <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi <WO prev=CO> bo

At this point the pattern succeeds, and we work back down the stack to
clean up, restoring as we go:

CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi 
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai 
CO     <CO prev=NULL> <WO>
NULL   

*******************************************************************/

#define ST st->u.curlyx

	case CURLYX:    /* start of /A*B/  (for complex A) */
	{
	    /* No need to save/restore up to this paren */
	    I32 parenfloor = scan->flags;
	    
	    assert(next); /* keep Coverity happy */
	    if (OP(PREVOPER(next)) == NOTHING) /* LONGJMP */
		next += ARG(next);

	    /* XXXX Probably it is better to teach regpush to support
	       parenfloor > PL_regsize... */
	    if (parenfloor > (I32)*PL_reglastparen)
		parenfloor = *PL_reglastparen; /* Pessimization... */

	    ST.prev_curlyx= cur_curlyx;
	    cur_curlyx = st;
	    ST.cp = PL_savestack_ix;

	    /* these fields contain the state of the current curly.
	     * they are accessed by subsequent WHILEMs */
	    ST.parenfloor = parenfloor;
	    ST.min = ARG1(scan);
	    ST.max = ARG2(scan);
	    ST.A = NEXTOPER(scan) + EXTRA_STEP_2ARGS;
	    ST.B = next;
	    ST.minmod = minmod;
	    minmod = 0;
	    ST.count = -1;	/* this will be updated by WHILEM */
	    ST.lastloc = NULL;  /* this will be updated by WHILEM */

	    PL_reginput = locinput;
	    PUSH_YES_STATE_GOTO(CURLYX_end, PREVOPER(next));
	    /* NOTREACHED */
	}

	case CURLYX_end: /* just finished matching all of A*B */
	    cur_curlyx = ST.prev_curlyx;
	    sayYES;
	    /* NOTREACHED */

	case CURLYX_end_fail: /* just failed to match all of A*B */
	    regcpblow(ST.cp);
	    cur_curlyx = ST.prev_curlyx;
	    sayNO;
	    /* NOTREACHED */


#undef ST
#define ST st->u.whilem

	case WHILEM:     /* just matched an A in /A*B/  (for complex A) */
	{
	    /* see the discussion above about CURLYX/WHILEM */
	    I32 n;
	    assert(cur_curlyx); /* keep Coverity happy */
	    n = ++cur_curlyx->u.curlyx.count; /* how many A's matched */
	    ST.save_lastloc = cur_curlyx->u.curlyx.lastloc;
	    ST.cache_offset = 0;
	    ST.cache_mask = 0;
	    
	    PL_reginput = locinput;

	    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
		  "%*s  whilem: matched %ld out of %ld..%ld\n",
		  REPORT_CODE_OFF+depth*2, "", (long)n,
		  (long)cur_curlyx->u.curlyx.min,
		  (long)cur_curlyx->u.curlyx.max)
	    );

	    /* First just match a string of min A's. */

	    if (n < cur_curlyx->u.curlyx.min) {
		cur_curlyx->u.curlyx.lastloc = locinput;
		PUSH_STATE_GOTO(WHILEM_A_pre, cur_curlyx->u.curlyx.A);
		/* NOTREACHED */
	    }

	    /* If degenerate A matches "", assume A done. */

	    if (locinput == cur_curlyx->u.curlyx.lastloc) {
		DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
		   "%*s  whilem: empty match detected, trying continuation...\n",
		   REPORT_CODE_OFF+depth*2, "")
		);
		goto do_whilem_B_max;
	    }

	    /* super-linear cache processing */

	    if (scan->flags) {

		if (!PL_reg_maxiter) {
		    /* start the countdown: Postpone detection until we
		     * know the match is not *that* much linear. */
		    PL_reg_maxiter = (PL_regeol - PL_bostr + 1) * (scan->flags>>4);
		    /* possible overflow for long strings and many CURLYX's */
		    if (PL_reg_maxiter < 0)
			PL_reg_maxiter = I32_MAX;
		    PL_reg_leftiter = PL_reg_maxiter;
		}

		if (PL_reg_leftiter-- == 0) {
		    /* initialise cache */
		    const I32 size = (PL_reg_maxiter + 7)/8;
		    if (PL_reg_poscache) {
			if ((I32)PL_reg_poscache_size < size) {
			    Renew(PL_reg_poscache, size, char);
			    PL_reg_poscache_size = size;
			}
			Zero(PL_reg_poscache, size, char);
		    }
		    else {
			PL_reg_poscache_size = size;
			Newxz(PL_reg_poscache, size, char);
		    }
		    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
      "%swhilem: Detected a super-linear match, switching on caching%s...\n",
			      PL_colors[4], PL_colors[5])
		    );
		}

		if (PL_reg_leftiter < 0) {
		    /* have we already failed at this position? */
		    I32 offset, mask;
		    offset  = (scan->flags & 0xf) - 1
		  		+ (locinput - PL_bostr)  * (scan->flags>>4);
		    mask    = 1 << (offset % 8);
		    offset /= 8;
		    if (PL_reg_poscache[offset] & mask) {
			DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
			    "%*s  whilem: (cache) already tried at this position...\n",
			    REPORT_CODE_OFF+depth*2, "")
			);
			sayNO; /* cache records failure */
		    }
		    ST.cache_offset = offset;
		    ST.cache_mask   = mask;
		}
	    }

	    /* Prefer B over A for minimal matching. */

	    if (cur_curlyx->u.curlyx.minmod) {
		ST.save_curlyx = cur_curlyx;
		cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
		ST.cp = regcppush(ST.save_curlyx->u.curlyx.parenfloor);
		REGCP_SET(ST.lastcp);
		PUSH_YES_STATE_GOTO(WHILEM_B_min, ST.save_curlyx->u.curlyx.B);
		/* NOTREACHED */
	    }

	    /* Prefer A over B for maximal matching. */

	    if (n < cur_curlyx->u.curlyx.max) { /* More greed allowed? */
		ST.cp = regcppush(cur_curlyx->u.curlyx.parenfloor);
		cur_curlyx->u.curlyx.lastloc = locinput;
		REGCP_SET(ST.lastcp);
		PUSH_STATE_GOTO(WHILEM_A_max, cur_curlyx->u.curlyx.A);
		/* NOTREACHED */
	    }
	    goto do_whilem_B_max;
	}
	/* NOTREACHED */

	case WHILEM_B_min: /* just matched B in a minimal match */
	case WHILEM_B_max: /* just matched B in a maximal match */
	    cur_curlyx = ST.save_curlyx;
	    sayYES;
	    /* NOTREACHED */

	case WHILEM_B_max_fail: /* just failed to match B in a maximal match */
	    cur_curlyx = ST.save_curlyx;
	    cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
	    cur_curlyx->u.curlyx.count--;
	    CACHEsayNO;
	    /* NOTREACHED */

	case WHILEM_A_min_fail: /* just failed to match A in a minimal match */
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex);
	    /* FALL THROUGH */
	case WHILEM_A_pre_fail: /* just failed to match even minimal A */
	    cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
	    cur_curlyx->u.curlyx.count--;
	    CACHEsayNO;
	    /* NOTREACHED */

	case WHILEM_A_max_fail: /* just failed to match A in a maximal match */
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex);	/* Restore some previous $<digit>s? */
	    PL_reginput = locinput;
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		"%*s  whilem: failed, trying continuation...\n",
		REPORT_CODE_OFF+depth*2, "")
	    );
	  do_whilem_B_max:
	    if (cur_curlyx->u.curlyx.count >= REG_INFTY
		&& ckWARN(WARN_REGEXP)
		&& !(PL_reg_flags & RF_warned))
	    {
		PL_reg_flags |= RF_warned;
		Perl_warner(aTHX_ packWARN(WARN_REGEXP), "%s limit (%d) exceeded",
		     "Complex regular subexpression recursion",
		     REG_INFTY - 1);
	    }

	    /* now try B */
	    ST.save_curlyx = cur_curlyx;
	    cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
	    PUSH_YES_STATE_GOTO(WHILEM_B_max, ST.save_curlyx->u.curlyx.B);
	    /* NOTREACHED */

	case WHILEM_B_min_fail: /* just failed to match B in a minimal match */
	    cur_curlyx = ST.save_curlyx;
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex);

	    if (cur_curlyx->u.curlyx.count >= cur_curlyx->u.curlyx.max) {
		/* Maximum greed exceeded */
		if (cur_curlyx->u.curlyx.count >= REG_INFTY
		    && ckWARN(WARN_REGEXP)
		    && !(PL_reg_flags & RF_warned))
		{
		    PL_reg_flags |= RF_warned;
		    Perl_warner(aTHX_ packWARN(WARN_REGEXP),
			"%s limit (%d) exceeded",
			"Complex regular subexpression recursion",
			REG_INFTY - 1);
		}
		cur_curlyx->u.curlyx.count--;
		CACHEsayNO;
	    }

	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		"%*s  trying longer...\n", REPORT_CODE_OFF+depth*2, "")
	    );
	    /* Try grabbing another A and see if it helps. */
	    PL_reginput = locinput;
	    cur_curlyx->u.curlyx.lastloc = locinput;
	    ST.cp = regcppush(cur_curlyx->u.curlyx.parenfloor);
	    REGCP_SET(ST.lastcp);
	    PUSH_STATE_GOTO(WHILEM_A_min, ST.save_curlyx->u.curlyx.A);
	    /* NOTREACHED */

#undef  ST
#define ST st->u.branch

	case BRANCHJ:	    /*  /(...|A|...)/ with long next pointer */
	    next = scan + ARG(scan);
	    if (next == scan)
		next = NULL;
	    scan = NEXTOPER(scan);
	    /* FALL THROUGH */

	case BRANCH:	    /*  /(...|A|...)/ */
	    scan = NEXTOPER(scan); /* scan now points to inner node */
	    ST.lastparen = *PL_reglastparen;
	    ST.next_branch = next;
	    REGCP_SET(ST.cp);
	    PL_reginput = locinput;

	    /* Now go into the branch */
	    if (has_cutgroup) {
	        PUSH_YES_STATE_GOTO(BRANCH_next, scan);    
	    } else {
	        PUSH_STATE_GOTO(BRANCH_next, scan);
	    }
	    /* NOTREACHED */
        case CUTGROUP:
            PL_reginput = locinput;
            sv_yes_mark = st->u.mark.mark_name = scan->flags ? NULL :
                MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
            PUSH_STATE_GOTO(CUTGROUP_next,next);
            /* NOTREACHED */
        case CUTGROUP_next_fail:
            do_cutgroup = 1;
            no_final = 1;
            if (st->u.mark.mark_name)
                sv_commit = st->u.mark.mark_name;
            sayNO;	    
            /* NOTREACHED */
        case BRANCH_next:
            sayYES;
            /* NOTREACHED */
	case BRANCH_next_fail: /* that branch failed; try the next, if any */
	    if (do_cutgroup) {
	        do_cutgroup = 0;
	        no_final = 0;
	    }
	    REGCP_UNWIND(ST.cp);
	    for (n = *PL_reglastparen; n > ST.lastparen; n--)
		PL_regoffs[n].end = -1;
	    *PL_reglastparen = n;
	    /*dmq: *PL_reglastcloseparen = n; */
	    scan = ST.next_branch;
	    /* no more branches? */
	    if (!scan || (OP(scan) != BRANCH && OP(scan) != BRANCHJ)) {
	        DEBUG_EXECUTE_r({
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sBRANCH failed...%s\n",
			REPORT_CODE_OFF+depth*2, "", 
			PL_colors[4],
			PL_colors[5] );
		});
		sayNO_SILENT;
            }
	    continue; /* execute next BRANCH[J] op */
	    /* NOTREACHED */
    
	case MINMOD:
	    minmod = 1;
	    break;

#undef  ST
#define ST st->u.curlym

	case CURLYM:	/* /A{m,n}B/ where A is fixed-length */

	    /* This is an optimisation of CURLYX that enables us to push
	     * only a single backtracking state, no matter how many matches
	     * there are in {m,n}. It relies on the pattern being constant
	     * length, with no parens to influence future backrefs
	     */

	    ST.me = scan;
	    scan = NEXTOPER(scan) + NODE_STEP_REGNODE;

	    /* if paren positive, emulate an OPEN/CLOSE around A */
	    if (ST.me->flags) {
		U32 paren = ST.me->flags;
		if (paren > PL_regsize)
		    PL_regsize = paren;
		if (paren > *PL_reglastparen)
		    *PL_reglastparen = paren;
		scan += NEXT_OFF(scan); /* Skip former OPEN. */
	    }
	    ST.A = scan;
	    ST.B = next;
	    ST.alen = 0;
	    ST.count = 0;
	    ST.minmod = minmod;
	    minmod = 0;
	    ST.c1 = CHRTEST_UNINIT;
	    REGCP_SET(ST.cp);

	    if (!(ST.minmod ? ARG1(ST.me) : ARG2(ST.me))) /* min/max */
		goto curlym_do_B;

	  curlym_do_A: /* execute the A in /A{m,n}B/  */
	    PL_reginput = locinput;
	    PUSH_YES_STATE_GOTO(CURLYM_A, ST.A); /* match A */
	    /* NOTREACHED */

	case CURLYM_A: /* we've just matched an A */
	    locinput = st->locinput;
	    nextchr = UCHARAT(locinput);

	    ST.count++;
	    /* after first match, determine A's length: u.curlym.alen */
	    if (ST.count == 1) {
		if (PL_reg_match_utf8) {
		    char *s = locinput;
		    while (s < PL_reginput) {
			ST.alen++;
			s += UTF8SKIP(s);
		    }
		}
		else {
		    ST.alen = PL_reginput - locinput;
		}
		if (ST.alen == 0)
		    ST.count = ST.minmod ? ARG1(ST.me) : ARG2(ST.me);
	    }
	    DEBUG_EXECUTE_r(
		PerlIO_printf(Perl_debug_log,
			  "%*s  CURLYM now matched %"IVdf" times, len=%"IVdf"...\n",
			  (int)(REPORT_CODE_OFF+(depth*2)), "",
			  (IV) ST.count, (IV)ST.alen)
	    );

	    locinput = PL_reginput;
	                
	    if (cur_eval && cur_eval->u.eval.close_paren && 
	        cur_eval->u.eval.close_paren == (U32)ST.me->flags) 
	        goto fake_end;
	        
	    {
		I32 max = (ST.minmod ? ARG1(ST.me) : ARG2(ST.me));
		if ( max == REG_INFTY || ST.count < max )
		    goto curlym_do_A; /* try to match another A */
	    }
	    goto curlym_do_B; /* try to match B */

	case CURLYM_A_fail: /* just failed to match an A */
	    REGCP_UNWIND(ST.cp);

	    if (ST.minmod || ST.count < ARG1(ST.me) /* min*/ 
	        || (cur_eval && cur_eval->u.eval.close_paren &&
	            cur_eval->u.eval.close_paren == (U32)ST.me->flags))
		sayNO;

	  curlym_do_B: /* execute the B in /A{m,n}B/  */
	    PL_reginput = locinput;
	    if (ST.c1 == CHRTEST_UNINIT) {
		/* calculate c1 and c2 for possible match of 1st char
		 * following curly */
		ST.c1 = ST.c2 = CHRTEST_VOID;
		if (HAS_TEXT(ST.B) || JUMPABLE(ST.B)) {
		    regnode *text_node = ST.B;
		    if (! HAS_TEXT(text_node))
			FIND_NEXT_IMPT(text_node);
	            /* this used to be 
	                
	                (HAS_TEXT(text_node) && PL_regkind[OP(text_node)] == EXACT)
	                
	            	But the former is redundant in light of the latter.
	            	
	            	if this changes back then the macro for 
	            	IS_TEXT and friends need to change.
	             */
		    if (PL_regkind[OP(text_node)] == EXACT)
		    {
		        
			ST.c1 = (U8)*STRING(text_node);
			ST.c2 =
			    (IS_TEXTF(text_node))
			    ? PL_fold[ST.c1]
			    : (IS_TEXTFL(text_node))
				? PL_fold_locale[ST.c1]
				: ST.c1;
		    }
		}
	    }

	    DEBUG_EXECUTE_r(
		PerlIO_printf(Perl_debug_log,
		    "%*s  CURLYM trying tail with matches=%"IVdf"...\n",
		    (int)(REPORT_CODE_OFF+(depth*2)),
		    "", (IV)ST.count)
		);
	    if (ST.c1 != CHRTEST_VOID
		    && UCHARAT(PL_reginput) != ST.c1
		    && UCHARAT(PL_reginput) != ST.c2)
	    {
		/* simulate B failing */
		DEBUG_OPTIMISE_r(
		    PerlIO_printf(Perl_debug_log,
		        "%*s  CURLYM Fast bail c1=%"IVdf" c2=%"IVdf"\n",
		        (int)(REPORT_CODE_OFF+(depth*2)),"",
		        (IV)ST.c1,(IV)ST.c2
		));
		state_num = CURLYM_B_fail;
		goto reenter_switch;
	    }

	    if (ST.me->flags) {
		/* mark current A as captured */
		I32 paren = ST.me->flags;
		if (ST.count) {
		    PL_regoffs[paren].start
			= HOPc(PL_reginput, -ST.alen) - PL_bostr;
		    PL_regoffs[paren].end = PL_reginput - PL_bostr;
		    /*dmq: *PL_reglastcloseparen = paren; */
		}
		else
		    PL_regoffs[paren].end = -1;
		if (cur_eval && cur_eval->u.eval.close_paren &&
		    cur_eval->u.eval.close_paren == (U32)ST.me->flags) 
		{
		    if (ST.count) 
	                goto fake_end;
	            else
	                sayNO;
	        }
	    }
	    
	    PUSH_STATE_GOTO(CURLYM_B, ST.B); /* match B */
	    /* NOTREACHED */

	case CURLYM_B_fail: /* just failed to match a B */
	    REGCP_UNWIND(ST.cp);
	    if (ST.minmod) {
		I32 max = ARG2(ST.me);
		if (max != REG_INFTY && ST.count == max)
		    sayNO;
		goto curlym_do_A; /* try to match a further A */
	    }
	    /* backtrack one A */
	    if (ST.count == ARG1(ST.me) /* min */)
		sayNO;
	    ST.count--;
	    locinput = HOPc(locinput, -ST.alen);
	    goto curlym_do_B; /* try to match B */

#undef ST
#define ST st->u.curly

#define CURLY_SETPAREN(paren, success) \
    if (paren) { \
	if (success) { \
	    PL_regoffs[paren].start = HOPc(locinput, -1) - PL_bostr; \
	    PL_regoffs[paren].end = locinput - PL_bostr; \
	    *PL_reglastcloseparen = paren; \
	} \
	else \
	    PL_regoffs[paren].end = -1; \
    }

	case STAR:		/*  /A*B/ where A is width 1 */
	    ST.paren = 0;
	    ST.min = 0;
	    ST.max = REG_INFTY;
	    scan = NEXTOPER(scan);
	    goto repeat;
	case PLUS:		/*  /A+B/ where A is width 1 */
	    ST.paren = 0;
	    ST.min = 1;
	    ST.max = REG_INFTY;
	    scan = NEXTOPER(scan);
	    goto repeat;
	case CURLYN:		/*  /(A){m,n}B/ where A is width 1 */
	    ST.paren = scan->flags;	/* Which paren to set */
	    if (ST.paren > PL_regsize)
		PL_regsize = ST.paren;
	    if (ST.paren > *PL_reglastparen)
		*PL_reglastparen = ST.paren;
	    ST.min = ARG1(scan);  /* min to match */
	    ST.max = ARG2(scan);  /* max to match */
	    if (cur_eval && cur_eval->u.eval.close_paren &&
	        cur_eval->u.eval.close_paren == (U32)ST.paren) {
	        ST.min=1;
	        ST.max=1;
	    }
            scan = regnext(NEXTOPER(scan) + NODE_STEP_REGNODE);
	    goto repeat;
	case CURLY:		/*  /A{m,n}B/ where A is width 1 */
	    ST.paren = 0;
	    ST.min = ARG1(scan);  /* min to match */
	    ST.max = ARG2(scan);  /* max to match */
	    scan = NEXTOPER(scan) + NODE_STEP_REGNODE;
	  repeat:
	    /*
	    * Lookahead to avoid useless match attempts
	    * when we know what character comes next.
	    *
	    * Used to only do .*x and .*?x, but now it allows
	    * for )'s, ('s and (?{ ... })'s to be in the way
	    * of the quantifier and the EXACT-like node.  -- japhy
	    */

	    if (ST.min > ST.max) /* XXX make this a compile-time check? */
		sayNO;
	    if (HAS_TEXT(next) || JUMPABLE(next)) {
		U8 *s;
		regnode *text_node = next;

		if (! HAS_TEXT(text_node)) 
		    FIND_NEXT_IMPT(text_node);

		if (! HAS_TEXT(text_node))
		    ST.c1 = ST.c2 = CHRTEST_VOID;
		else {
		    if ( PL_regkind[OP(text_node)] != EXACT ) {
			ST.c1 = ST.c2 = CHRTEST_VOID;
			goto assume_ok_easy;
		    }
		    else
			s = (U8*)STRING(text_node);
                    
                    /*  Currently we only get here when 
                        
                        PL_rekind[OP(text_node)] == EXACT
                    
                        if this changes back then the macro for IS_TEXT and 
                        friends need to change. */
		    if (!UTF) {
			ST.c2 = ST.c1 = *s;
			if (IS_TEXTF(text_node))
			    ST.c2 = PL_fold[ST.c1];
			else if (IS_TEXTFL(text_node))
			    ST.c2 = PL_fold_locale[ST.c1];
		    }
		    else { /* UTF */
			if (IS_TEXTF(text_node)) {
			     STRLEN ulen1, ulen2;
			     U8 tmpbuf1[UTF8_MAXBYTES_CASE+1];
			     U8 tmpbuf2[UTF8_MAXBYTES_CASE+1];

			     to_utf8_lower((U8*)s, tmpbuf1, &ulen1);
			     to_utf8_upper((U8*)s, tmpbuf2, &ulen2);
#ifdef EBCDIC
			     ST.c1 = utf8n_to_uvchr(tmpbuf1, UTF8_MAXLEN, 0,
						    ckWARN(WARN_UTF8) ?
                                                    0 : UTF8_ALLOW_ANY);
			     ST.c2 = utf8n_to_uvchr(tmpbuf2, UTF8_MAXLEN, 0,
                                                    ckWARN(WARN_UTF8) ?
                                                    0 : UTF8_ALLOW_ANY);
#else
			     ST.c1 = utf8n_to_uvuni(tmpbuf1, UTF8_MAXBYTES, 0,
						    uniflags);
			     ST.c2 = utf8n_to_uvuni(tmpbuf2, UTF8_MAXBYTES, 0,
						    uniflags);
#endif
			}
			else {
			    ST.c2 = ST.c1 = utf8n_to_uvchr(s, UTF8_MAXBYTES, 0,
						     uniflags);
			}
		    }
		}
	    }
	    else
		ST.c1 = ST.c2 = CHRTEST_VOID;
	assume_ok_easy:

	    ST.A = scan;
	    ST.B = next;
	    PL_reginput = locinput;
	    if (minmod) {
		minmod = 0;
		if (ST.min && regrepeat(rex, ST.A, ST.min, depth) < ST.min)
		    sayNO;
		ST.count = ST.min;
		locinput = PL_reginput;
		REGCP_SET(ST.cp);
		if (ST.c1 == CHRTEST_VOID)
		    goto curly_try_B_min;

		ST.oldloc = locinput;

		/* set ST.maxpos to the furthest point along the
		 * string that could possibly match */
		if  (ST.max == REG_INFTY) {
		    ST.maxpos = PL_regeol - 1;
		    if (do_utf8)
			while (UTF8_IS_CONTINUATION(*(U8*)ST.maxpos))
			    ST.maxpos--;
		}
		else if (do_utf8) {
		    int m = ST.max - ST.min;
		    for (ST.maxpos = locinput;
			 m >0 && ST.maxpos + UTF8SKIP(ST.maxpos) <= PL_regeol; m--)
			ST.maxpos += UTF8SKIP(ST.maxpos);
		}
		else {
		    ST.maxpos = locinput + ST.max - ST.min;
		    if (ST.maxpos >= PL_regeol)
			ST.maxpos = PL_regeol - 1;
		}
		goto curly_try_B_min_known;

	    }
	    else {
		ST.count = regrepeat(rex, ST.A, ST.max, depth);
		locinput = PL_reginput;
		if (ST.count < ST.min)
		    sayNO;
		if ((ST.count > ST.min)
		    && (PL_regkind[OP(ST.B)] == EOL) && (OP(ST.B) != MEOL))
		{
		    /* A{m,n} must come at the end of the string, there's
		     * no point in backing off ... */
		    ST.min = ST.count;
		    /* ...except that $ and \Z can match before *and* after
		       newline at the end.  Consider "\n\n" =~ /\n+\Z\n/.
		       We may back off by one in this case. */
		    if (UCHARAT(PL_reginput - 1) == '\n' && OP(ST.B) != EOS)
			ST.min--;
		}
		REGCP_SET(ST.cp);
		goto curly_try_B_max;
	    }
	    /* NOTREACHED */


	case CURLY_B_min_known_fail:
	    /* failed to find B in a non-greedy match where c1,c2 valid */
	    if (ST.paren && ST.count)
		PL_regoffs[ST.paren].end = -1;

	    PL_reginput = locinput;	/* Could be reset... */
	    REGCP_UNWIND(ST.cp);
	    /* Couldn't or didn't -- move forward. */
	    ST.oldloc = locinput;
	    if (do_utf8)
		locinput += UTF8SKIP(locinput);
	    else
		locinput++;
	    ST.count++;
	  curly_try_B_min_known:
	     /* find the next place where 'B' could work, then call B */
	    {
		int n;
		if (do_utf8) {
		    n = (ST.oldloc == locinput) ? 0 : 1;
		    if (ST.c1 == ST.c2) {
			STRLEN len;
			/* set n to utf8_distance(oldloc, locinput) */
			while (locinput <= ST.maxpos &&
			       utf8n_to_uvchr((U8*)locinput,
					      UTF8_MAXBYTES, &len,
					      uniflags) != (UV)ST.c1) {
			    locinput += len;
			    n++;
			}
		    }
		    else {
			/* set n to utf8_distance(oldloc, locinput) */
			while (locinput <= ST.maxpos) {
			    STRLEN len;
			    const UV c = utf8n_to_uvchr((U8*)locinput,
						  UTF8_MAXBYTES, &len,
						  uniflags);
			    if (c == (UV)ST.c1 || c == (UV)ST.c2)
				break;
			    locinput += len;
			    n++;
			}
		    }
		}
		else {
		    if (ST.c1 == ST.c2) {
			while (locinput <= ST.maxpos &&
			       UCHARAT(locinput) != ST.c1)
			    locinput++;
		    }
		    else {
			while (locinput <= ST.maxpos
			       && UCHARAT(locinput) != ST.c1
			       && UCHARAT(locinput) != ST.c2)
			    locinput++;
		    }
		    n = locinput - ST.oldloc;
		}
		if (locinput > ST.maxpos)
		    sayNO;
		/* PL_reginput == oldloc now */
		if (n) {
		    ST.count += n;
		    if (regrepeat(rex, ST.A, n, depth) < n)
			sayNO;
		}
		PL_reginput = locinput;
		CURLY_SETPAREN(ST.paren, ST.count);
		if (cur_eval && cur_eval->u.eval.close_paren && 
		    cur_eval->u.eval.close_paren == (U32)ST.paren) {
		    goto fake_end;
	        }
		PUSH_STATE_GOTO(CURLY_B_min_known, ST.B);
	    }
	    /* NOTREACHED */


	case CURLY_B_min_fail:
	    /* failed to find B in a non-greedy match where c1,c2 invalid */
	    if (ST.paren && ST.count)
		PL_regoffs[ST.paren].end = -1;

	    REGCP_UNWIND(ST.cp);
	    /* failed -- move forward one */
	    PL_reginput = locinput;
	    if (regrepeat(rex, ST.A, 1, depth)) {
		ST.count++;
		locinput = PL_reginput;
		if (ST.count <= ST.max || (ST.max == REG_INFTY &&
			ST.count > 0)) /* count overflow ? */
		{
		  curly_try_B_min:
		    CURLY_SETPAREN(ST.paren, ST.count);
		    if (cur_eval && cur_eval->u.eval.close_paren &&
		        cur_eval->u.eval.close_paren == (U32)ST.paren) {
                        goto fake_end;
                    }
		    PUSH_STATE_GOTO(CURLY_B_min, ST.B);
		}
	    }
	    sayNO;
	    /* NOTREACHED */


	curly_try_B_max:
	    /* a successful greedy match: now try to match B */
            if (cur_eval && cur_eval->u.eval.close_paren &&
                cur_eval->u.eval.close_paren == (U32)ST.paren) {
                goto fake_end;
            }
	    {
		UV c = 0;
		if (ST.c1 != CHRTEST_VOID)
		    c = do_utf8 ? utf8n_to_uvchr((U8*)PL_reginput,
					   UTF8_MAXBYTES, 0, uniflags)
				: (UV) UCHARAT(PL_reginput);
		/* If it could work, try it. */
		if (ST.c1 == CHRTEST_VOID || c == (UV)ST.c1 || c == (UV)ST.c2) {
		    CURLY_SETPAREN(ST.paren, ST.count);
		    PUSH_STATE_GOTO(CURLY_B_max, ST.B);
		    /* NOTREACHED */
		}
	    }
	    /* FALL THROUGH */
	case CURLY_B_max_fail:
	    /* failed to find B in a greedy match */
	    if (ST.paren && ST.count)
		PL_regoffs[ST.paren].end = -1;

	    REGCP_UNWIND(ST.cp);
	    /*  back up. */
	    if (--ST.count < ST.min)
		sayNO;
	    PL_reginput = locinput = HOPc(locinput, -1);
	    goto curly_try_B_max;

#undef ST

	case END:
	    fake_end:
	    if (cur_eval) {
		/* we've just finished A in /(??{A})B/; now continue with B */
		I32 tmpix;
		st->u.eval.toggle_reg_flags
			    = cur_eval->u.eval.toggle_reg_flags;
		PL_reg_flags ^= st->u.eval.toggle_reg_flags; 

		st->u.eval.prev_rex = rex_sv;		/* inner */
		SETREX(rex_sv,cur_eval->u.eval.prev_rex);
		rex = (struct regexp *)SvANY(rex_sv);
		rexi = RXi_GET(rex);
		cur_curlyx = cur_eval->u.eval.prev_curlyx;
		ReREFCNT_inc(rex_sv);
		st->u.eval.cp = regcppush(0);	/* Save *all* the positions. */

		/* rex was changed so update the pointer in PL_reglastparen and PL_reglastcloseparen */
		PL_reglastparen = &rex->lastparen;
		PL_reglastcloseparen = &rex->lastcloseparen;

		REGCP_SET(st->u.eval.lastcp);
		PL_reginput = locinput;

		/* Restore parens of the outer rex without popping the
		 * savestack */
		tmpix = PL_savestack_ix;
		PL_savestack_ix = cur_eval->u.eval.lastcp;
		regcppop(rex);
		PL_savestack_ix = tmpix;

		st->u.eval.prev_eval = cur_eval;
		cur_eval = cur_eval->u.eval.prev_eval;
		DEBUG_EXECUTE_r(
		    PerlIO_printf(Perl_debug_log, "%*s  EVAL trying tail ... %"UVxf"\n",
				      REPORT_CODE_OFF+depth*2, "",PTR2UV(cur_eval)););
                if ( nochange_depth )
	            nochange_depth--;

                PUSH_YES_STATE_GOTO(EVAL_AB,
			st->u.eval.prev_eval->u.eval.B); /* match B */
	    }

	    if (locinput < reginfo->till) {
		DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
				      "%sMatch possible, but length=%ld is smaller than requested=%ld, failing!%s\n",
				      PL_colors[4],
				      (long)(locinput - PL_reg_starttry),
				      (long)(reginfo->till - PL_reg_starttry),
				      PL_colors[5]));
               				      
		sayNO_SILENT;		/* Cannot match: too short. */
	    }
	    PL_reginput = locinput;	/* put where regtry can find it */
	    sayYES;			/* Success! */

	case SUCCEED: /* successful SUSPEND/UNLESSM/IFMATCH/CURLYM */
	    DEBUG_EXECUTE_r(
	    PerlIO_printf(Perl_debug_log,
		"%*s  %ssubpattern success...%s\n",
		REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5]));
	    PL_reginput = locinput;	/* put where regtry can find it */
	    sayYES;			/* Success! */

#undef  ST
#define ST st->u.ifmatch

	case SUSPEND:	/* (?>A) */
	    ST.wanted = 1;
	    PL_reginput = locinput;
	    goto do_ifmatch;	

	case UNLESSM:	/* -ve lookaround: (?!A), or with flags, (?<!A) */
	    ST.wanted = 0;
	    goto ifmatch_trivial_fail_test;

	case IFMATCH:	/* +ve lookaround: (?=A), or with flags, (?<=A) */
	    ST.wanted = 1;
	  ifmatch_trivial_fail_test:
	    if (scan->flags) {
		char * const s = HOPBACKc(locinput, scan->flags);
		if (!s) {
		    /* trivial fail */
		    if (logical) {
			logical = 0;
			sw = 1 - (bool)ST.wanted;
		    }
		    else if (ST.wanted)
			sayNO;
		    next = scan + ARG(scan);
		    if (next == scan)
			next = NULL;
		    break;
		}
		PL_reginput = s;
	    }
	    else
		PL_reginput = locinput;

	  do_ifmatch:
	    ST.me = scan;
	    ST.logical = logical;
	    logical = 0; /* XXX: reset state of logical once it has been saved into ST */
	    
	    /* execute body of (?...A) */
	    PUSH_YES_STATE_GOTO(IFMATCH_A, NEXTOPER(NEXTOPER(scan)));
	    /* NOTREACHED */

	case IFMATCH_A_fail: /* body of (?...A) failed */
	    ST.wanted = !ST.wanted;
	    /* FALL THROUGH */

	case IFMATCH_A: /* body of (?...A) succeeded */
	    if (ST.logical) {
		sw = (bool)ST.wanted;
	    }
	    else if (!ST.wanted)
		sayNO;

	    if (OP(ST.me) == SUSPEND)
		locinput = PL_reginput;
	    else {
		locinput = PL_reginput = st->locinput;
		nextchr = UCHARAT(locinput);
	    }
	    scan = ST.me + ARG(ST.me);
	    if (scan == ST.me)
		scan = NULL;
	    continue; /* execute B */

#undef ST

	case LONGJMP:
	    next = scan + ARG(scan);
	    if (next == scan)
		next = NULL;
	    break;
	case COMMIT:
	    reginfo->cutpoint = PL_regeol;
	    /* FALLTHROUGH */
	case PRUNE:
	    PL_reginput = locinput;
	    if (!scan->flags)
	        sv_yes_mark = sv_commit = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
	    PUSH_STATE_GOTO(COMMIT_next,next);
	    /* NOTREACHED */
	case COMMIT_next_fail:
	    no_final = 1;    
	    /* FALLTHROUGH */	    
	case OPFAIL:
	    sayNO;
	    /* NOTREACHED */

#define ST st->u.mark
        case MARKPOINT:
            ST.prev_mark = mark_state;
            ST.mark_name = sv_commit = sv_yes_mark 
                = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
            mark_state = st;
            ST.mark_loc = PL_reginput = locinput;
            PUSH_YES_STATE_GOTO(MARKPOINT_next,next);
            /* NOTREACHED */
        case MARKPOINT_next:
            mark_state = ST.prev_mark;
            sayYES;
            /* NOTREACHED */
        case MARKPOINT_next_fail:
            if (popmark && sv_eq(ST.mark_name,popmark)) 
            {
                if (ST.mark_loc > startpoint)
	            reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
                popmark = NULL; /* we found our mark */
                sv_commit = ST.mark_name;

                DEBUG_EXECUTE_r({
                        PerlIO_printf(Perl_debug_log,
		            "%*s  %ssetting cutpoint to mark:%"SVf"...%s\n",
		            REPORT_CODE_OFF+depth*2, "", 
		            PL_colors[4], SVfARG(sv_commit), PL_colors[5]);
		});
            }
            mark_state = ST.prev_mark;
            sv_yes_mark = mark_state ? 
                mark_state->u.mark.mark_name : NULL;
            sayNO;
            /* NOTREACHED */
        case SKIP:
            PL_reginput = locinput;
            if (scan->flags) {
                /* (*SKIP) : if we fail we cut here*/
                ST.mark_name = NULL;
                ST.mark_loc = locinput;
                PUSH_STATE_GOTO(SKIP_next,next);    
            } else {
                /* (*SKIP:NAME) : if there is a (*MARK:NAME) fail where it was, 
                   otherwise do nothing.  Meaning we need to scan 
                 */
                regmatch_state *cur = mark_state;
                SV *find = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
                
                while (cur) {
                    if ( sv_eq( cur->u.mark.mark_name, 
                                find ) ) 
                    {
                        ST.mark_name = find;
                        PUSH_STATE_GOTO( SKIP_next, next );
                    }
                    cur = cur->u.mark.prev_mark;
                }
            }    
            /* Didn't find our (*MARK:NAME) so ignore this (*SKIP:NAME) */
            break;    
	case SKIP_next_fail:
	    if (ST.mark_name) {
	        /* (*CUT:NAME) - Set up to search for the name as we 
	           collapse the stack*/
	        popmark = ST.mark_name;	   
	    } else {
	        /* (*CUT) - No name, we cut here.*/
	        if (ST.mark_loc > startpoint)
	            reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
	        /* but we set sv_commit to latest mark_name if there
	           is one so they can test to see how things lead to this
	           cut */    
                if (mark_state) 
                    sv_commit=mark_state->u.mark.mark_name;	            
            } 
            no_final = 1; 
            sayNO;
            /* NOTREACHED */
#undef ST
        case FOLDCHAR:
            n = ARG(scan);
            if ( n == (U32)what_len_TRICKYFOLD(locinput,do_utf8,ln) ) {
                locinput += ln;
            } else if ( 0xDF == n && !do_utf8 && !UTF ) {
                sayNO;
            } else  {
                U8 folded[UTF8_MAXBYTES_CASE+1];
                STRLEN foldlen;
                const char * const l = locinput;
                char *e = PL_regeol;
                to_uni_fold(n, folded, &foldlen);

		if (ibcmp_utf8((const char*) folded, 0,  foldlen, 1,
                	       l, &e, 0,  do_utf8)) {
                        sayNO;
                }
                locinput = e;
            } 
            nextchr = UCHARAT(locinput);  
            break;
        case LNBREAK:
            if ((n=is_LNBREAK(locinput,do_utf8))) {
                locinput += n;
                nextchr = UCHARAT(locinput);
            } else
                sayNO;
            break;

#define CASE_CLASS(nAmE)                              \
        case nAmE:                                    \
            if ((n=is_##nAmE(locinput,do_utf8))) {    \
                locinput += n;                        \
                nextchr = UCHARAT(locinput);          \
            } else                                    \
                sayNO;                                \
            break;                                    \
        case N##nAmE:                                 \
            if ((n=is_##nAmE(locinput,do_utf8))) {    \
                sayNO;                                \
            } else {                                  \
                locinput += UTF8SKIP(locinput);       \
                nextchr = UCHARAT(locinput);          \
            }                                         \
            break

        CASE_CLASS(VERTWS);
        CASE_CLASS(HORIZWS);
#undef CASE_CLASS

	default:
	    PerlIO_printf(Perl_error_log, "%"UVxf" %d\n",
			  PTR2UV(scan), OP(scan));
	    Perl_croak(aTHX_ "regexp memory corruption");
	    
	} /* end switch */ 

        /* switch break jumps here */
	scan = next; /* prepare to execute the next op and ... */
	continue;    /* ... jump back to the top, reusing st */
	/* NOTREACHED */

      push_yes_state:
	/* push a state that backtracks on success */
	st->u.yes.prev_yes_state = yes_state;
	yes_state = st;
	/* FALL THROUGH */
      push_state:
	/* push a new regex state, then continue at scan  */
	{
	    regmatch_state *newst;

	    DEBUG_STACK_r({
	        regmatch_state *cur = st;
	        regmatch_state *curyes = yes_state;
	        int curd = depth;
	        regmatch_slab *slab = PL_regmatch_slab;
                for (;curd > -1;cur--,curd--) {
                    if (cur < SLAB_FIRST(slab)) {
                	slab = slab->prev;
                	cur = SLAB_LAST(slab);
                    }
                    PerlIO_printf(Perl_error_log, "%*s#%-3d %-10s %s\n",
                        REPORT_CODE_OFF + 2 + depth * 2,"",
                        curd, PL_reg_name[cur->resume_state],
                        (curyes == cur) ? "yes" : ""
                    );
                    if (curyes == cur)
	                curyes = cur->u.yes.prev_yes_state;
                }
            } else 
                DEBUG_STATE_pp("push")
            );
	    depth++;
	    st->locinput = locinput;
	    newst = st+1; 
	    if (newst >  SLAB_LAST(PL_regmatch_slab))
		newst = S_push_slab(aTHX);
	    PL_regmatch_state = newst;

	    locinput = PL_reginput;
	    nextchr = UCHARAT(locinput);
	    st = newst;
	    continue;
	    /* NOTREACHED */
	}
    }

    /*
    * We get here only if there's trouble -- normally "case END" is
    * the terminating point.
    */
    Perl_croak(aTHX_ "corrupted regexp pointers");
    /*NOTREACHED*/
    sayNO;

yes:
    if (yes_state) {
	/* we have successfully completed a subexpression, but we must now
	 * pop to the state marked by yes_state and continue from there */
	assert(st != yes_state);
#ifdef DEBUGGING
	while (st != yes_state) {
	    st--;
	    if (st < SLAB_FIRST(PL_regmatch_slab)) {
		PL_regmatch_slab = PL_regmatch_slab->prev;
		st = SLAB_LAST(PL_regmatch_slab);
	    }
	    DEBUG_STATE_r({
	        if (no_final) {
	            DEBUG_STATE_pp("pop (no final)");        
	        } else {
	            DEBUG_STATE_pp("pop (yes)");
	        }
	    });
	    depth--;
	}
#else
	while (yes_state < SLAB_FIRST(PL_regmatch_slab)
	    || yes_state > SLAB_LAST(PL_regmatch_slab))
	{
	    /* not in this slab, pop slab */
	    depth -= (st - SLAB_FIRST(PL_regmatch_slab) + 1);
	    PL_regmatch_slab = PL_regmatch_slab->prev;
	    st = SLAB_LAST(PL_regmatch_slab);
	}
	depth -= (st - yes_state);
#endif
	st = yes_state;
	yes_state = st->u.yes.prev_yes_state;
	PL_regmatch_state = st;
        
        if (no_final) {
            locinput= st->locinput;
            nextchr = UCHARAT(locinput);
        }
	state_num = st->resume_state + no_final;
	goto reenter_switch;
    }

    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%sMatch successful!%s\n",
			  PL_colors[4], PL_colors[5]));

    if (PL_reg_eval_set) {
	/* each successfully executed (?{...}) block does the equivalent of
	 *   local $^R = do {...}
	 * When popping the save stack, all these locals would be undone;
	 * bypass this by setting the outermost saved $^R to the latest
	 * value */
	if (oreplsv != GvSV(PL_replgv))
	    sv_setsv(oreplsv, GvSV(PL_replgv));
    }
    result = 1;
    goto final_exit;

no:
    DEBUG_EXECUTE_r(
	PerlIO_printf(Perl_debug_log,
            "%*s  %sfailed...%s\n",
            REPORT_CODE_OFF+depth*2, "", 
            PL_colors[4], PL_colors[5])
	);

no_silent:
    if (no_final) {
        if (yes_state) {
            goto yes;
        } else {
            goto final_exit;
        }
    }    
    if (depth) {
	/* there's a previous state to backtrack to */
	st--;
	if (st < SLAB_FIRST(PL_regmatch_slab)) {
	    PL_regmatch_slab = PL_regmatch_slab->prev;
	    st = SLAB_LAST(PL_regmatch_slab);
	}
	PL_regmatch_state = st;
	locinput= st->locinput;
	nextchr = UCHARAT(locinput);

	DEBUG_STATE_pp("pop");
	depth--;
	if (yes_state == st)
	    yes_state = st->u.yes.prev_yes_state;

	state_num = st->resume_state + 1; /* failure = success + 1 */
	goto reenter_switch;
    }
    result = 0;

  final_exit:
    if (rex->intflags & PREGf_VERBARG_SEEN) {
        SV *sv_err = get_sv("REGERROR", 1);
        SV *sv_mrk = get_sv("REGMARK", 1);
        if (result) {
            sv_commit = &PL_sv_no;
            if (!sv_yes_mark) 
                sv_yes_mark = &PL_sv_yes;
        } else {
            if (!sv_commit) 
                sv_commit = &PL_sv_yes;
            sv_yes_mark = &PL_sv_no;
        }
        sv_setsv(sv_err, sv_commit);
        sv_setsv(sv_mrk, sv_yes_mark);
    }

    /* clean up; in particular, free all slabs above current one */
    LEAVE_SCOPE(oldsave);

    return result;
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
/*
 * [This routine now assumes that it will only match on things of length 1.
 * That was true before, but now we assume scan - reginput is the count,
 * rather than incrementing count on every character.  [Er, except utf8.]]
 */
STATIC I32
S_regrepeat(pTHX_ const regexp *prog, const regnode *p, I32 max, int depth)
{
    dVAR;
    register char *scan;
    register I32 c;
    register char *loceol = PL_regeol;
    register I32 hardcount = 0;
    register bool do_utf8 = PL_reg_match_utf8;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    PERL_ARGS_ASSERT_REGREPEAT;

    scan = PL_reginput;
    if (max == REG_INFTY)
	max = I32_MAX;
    else if (max < loceol - scan)
	loceol = scan + max;
    switch (OP(p)) {
    case REG_ANY:
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (scan < loceol && hardcount < max && *scan != '\n') {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && *scan != '\n')
		scan++;
	}
	break;
    case SANY:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (scan < loceol && hardcount < max) {
	        scan += UTF8SKIP(scan);
		hardcount++;
	    }
	}
	else
	    scan = loceol;
	break;
    case CANY:
	scan = loceol;
	break;
    case EXACT:		/* length of string is 1 */
	c = (U8)*STRING(p);
	while (scan < loceol && UCHARAT(scan) == c)
	    scan++;
	break;
    case EXACTF:	/* length of string is 1 */
	c = (U8)*STRING(p);
	while (scan < loceol &&
	       (UCHARAT(scan) == c || UCHARAT(scan) == PL_fold[c]))
	    scan++;
	break;
    case EXACTFL:	/* length of string is 1 */
	PL_reg_flags |= RF_tainted;
	c = (U8)*STRING(p);
	while (scan < loceol &&
	       (UCHARAT(scan) == c || UCHARAT(scan) == PL_fold_locale[c]))
	    scan++;
	break;
    case ANYOF:
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol &&
		   reginclass(prog, p, (U8*)scan, 0, do_utf8)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && REGINCLASS(prog, p, (U8*)scan))
		scan++;
	}
	break;
    case ALNUM:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_ALNUM();
	    while (hardcount < max && scan < loceol &&
		   swash_fetch(PL_utf8_alnum, (U8*)scan, do_utf8)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && isALNUM(*scan))
		scan++;
	}
	break;
    case ALNUML:
	PL_reg_flags |= RF_tainted;
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol &&
		   isALNUM_LC_utf8((U8*)scan)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && isALNUM_LC(*scan))
		scan++;
	}
	break;
    case NALNUM:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_ALNUM();
	    while (hardcount < max && scan < loceol &&
		   !swash_fetch(PL_utf8_alnum, (U8*)scan, do_utf8)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !isALNUM(*scan))
		scan++;
	}
	break;
    case NALNUML:
	PL_reg_flags |= RF_tainted;
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol &&
		   !isALNUM_LC_utf8((U8*)scan)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !isALNUM_LC(*scan))
		scan++;
	}
	break;
    case SPACE:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_SPACE();
	    while (hardcount < max && scan < loceol &&
		   (*scan == ' ' ||
		    swash_fetch(PL_utf8_space,(U8*)scan, do_utf8))) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && isSPACE(*scan))
		scan++;
	}
	break;
    case SPACEL:
	PL_reg_flags |= RF_tainted;
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol &&
		   (*scan == ' ' || isSPACE_LC_utf8((U8*)scan))) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && isSPACE_LC(*scan))
		scan++;
	}
	break;
    case NSPACE:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_SPACE();
	    while (hardcount < max && scan < loceol &&
		   !(*scan == ' ' ||
		     swash_fetch(PL_utf8_space,(U8*)scan, do_utf8))) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !isSPACE(*scan))
		scan++;
	}
	break;
    case NSPACEL:
	PL_reg_flags |= RF_tainted;
	if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol &&
		   !(*scan == ' ' || isSPACE_LC_utf8((U8*)scan))) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !isSPACE_LC(*scan))
		scan++;
	}
	break;
    case DIGIT:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_DIGIT();
	    while (hardcount < max && scan < loceol &&
		   swash_fetch(PL_utf8_digit, (U8*)scan, do_utf8)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && isDIGIT(*scan))
		scan++;
	}
	break;
    case NDIGIT:
	if (do_utf8) {
	    loceol = PL_regeol;
	    LOAD_UTF8_CHARCLASS_DIGIT();
	    while (hardcount < max && scan < loceol &&
		   !swash_fetch(PL_utf8_digit, (U8*)scan, do_utf8)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !isDIGIT(*scan))
		scan++;
	}
    case LNBREAK:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol && (c=is_LNBREAK_utf8(scan))) {
		scan += c;
		hardcount++;
	    }
	} else {
	    /*
	      LNBREAK can match two latin chars, which is ok,
	      because we have a null terminated string, but we
	      have to use hardcount in this situation
	    */
	    while (scan < loceol && (c=is_LNBREAK_latin1(scan)))  {
		scan+=c;
		hardcount++;
	    }
	}	
	break;
    case HORIZWS:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol && (c=is_HORIZWS_utf8(scan))) {
		scan += c;
		hardcount++;
	    }
	} else {
	    while (scan < loceol && is_HORIZWS_latin1(scan)) 
		scan++;		
	}	
	break;
    case NHORIZWS:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol && !is_HORIZWS_utf8(scan)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !is_HORIZWS_latin1(scan))
		scan++;

	}	
	break;
    case VERTWS:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol && (c=is_VERTWS_utf8(scan))) {
		scan += c;
		hardcount++;
	    }
	} else {
	    while (scan < loceol && is_VERTWS_latin1(scan)) 
		scan++;

	}	
	break;
    case NVERTWS:
        if (do_utf8) {
	    loceol = PL_regeol;
	    while (hardcount < max && scan < loceol && !is_VERTWS_utf8(scan)) {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && !is_VERTWS_latin1(scan)) 
		scan++;
          
	}	
	break;

    default:		/* Called on something of 0 width. */
	break;		/* So match right here or not at all. */
    }

    if (hardcount)
	c = hardcount;
    else
	c = scan - PL_reginput;
    PL_reginput = scan;

    DEBUG_r({
	GET_RE_DEBUG_FLAGS_DECL;
	DEBUG_EXECUTE_r({
	    SV * const prop = sv_newmortal();
	    regprop(prog, prop, p);
	    PerlIO_printf(Perl_debug_log,
			"%*s  %s can match %"IVdf" times out of %"IVdf"...\n",
			REPORT_CODE_OFF + depth*2, "", SvPVX_const(prop),(IV)c,(IV)max);
	});
    });

    return(c);
}


#if !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION)
/*
- regclass_swash - prepare the utf8 swash
*/

SV *
Perl_regclass_swash(pTHX_ const regexp *prog, register const regnode* node, bool doinit, SV** listsvp, SV **altsvp)
{
    dVAR;
    SV *sw  = NULL;
    SV *si  = NULL;
    SV *alt = NULL;
    RXi_GET_DECL(prog,progi);
    const struct reg_data * const data = prog ? progi->data : NULL;

    PERL_ARGS_ASSERT_REGCLASS_SWASH;

    if (data && data->count) {
	const U32 n = ARG(node);

	if (data->what[n] == 's') {
	    SV * const rv = MUTABLE_SV(data->data[n]);
	    AV * const av = MUTABLE_AV(SvRV(rv));
	    SV **const ary = AvARRAY(av);
	    SV **a, **b;
	
	    /* See the end of regcomp.c:S_regclass() for
	     * documentation of these array elements. */

	    si = *ary;
	    a  = SvROK(ary[1]) ? &ary[1] : NULL;
	    b  = SvTYPE(ary[2]) == SVt_PVAV ? &ary[2] : NULL;

	    if (a)
		sw = *a;
	    else if (si && doinit) {
		sw = swash_init("utf8", "", si, 1, 0);
		(void)av_store(av, 1, sw);
	    }
	    if (b)
	        alt = *b;
	}
    }
	
    if (listsvp)
	*listsvp = si;
    if (altsvp)
	*altsvp  = alt;

    return sw;
}
#endif

/*
 - reginclass - determine if a character falls into a character class
 
  The n is the ANYOF regnode, the p is the target string, lenp
  is pointer to the maximum length of how far to go in the p
  (if the lenp is zero, UTF8SKIP(p) is used),
  do_utf8 tells whether the target string is in UTF-8.

 */

STATIC bool
S_reginclass(pTHX_ const regexp *prog, register const regnode *n, register const U8* p, STRLEN* lenp, register bool do_utf8)
{
    dVAR;
    const char flags = ANYOF_FLAGS(n);
    bool match = FALSE;
    UV c = *p;
    STRLEN len = 0;
    STRLEN plen;

    PERL_ARGS_ASSERT_REGINCLASS;

    if (do_utf8 && !UTF8_IS_INVARIANT(c)) {
	c = utf8n_to_uvchr(p, UTF8_MAXBYTES, &len,
		(UTF8_ALLOW_DEFAULT & UTF8_ALLOW_ANYUV)
		| UTF8_ALLOW_FFFF | UTF8_CHECK_ONLY);
		/* see [perl #37836] for UTF8_ALLOW_ANYUV; [perl #38293] for
		 * UTF8_ALLOW_FFFF */
	if (len == (STRLEN)-1) 
	    Perl_croak(aTHX_ "Malformed UTF-8 character (fatal)");
    }

    plen = lenp ? *lenp : UNISKIP(NATIVE_TO_UNI(c));
    if (do_utf8 || (flags & ANYOF_UNICODE)) {
        if (lenp)
	    *lenp = 0;
	if (do_utf8 && !ANYOF_RUNTIME(n)) {
	    if (len != (STRLEN)-1 && c < 256 && ANYOF_BITMAP_TEST(n, c))
		match = TRUE;
	}
	if (!match && do_utf8 && (flags & ANYOF_UNICODE_ALL) && c >= 256)
	    match = TRUE;
	if (!match) {
	    AV *av;
	    SV * const sw = regclass_swash(prog, n, TRUE, 0, (SV**)&av);
	
	    if (sw) {
		U8 * utf8_p;
		if (do_utf8) {
		    utf8_p = (U8 *) p;
		} else {
		    STRLEN len = 1;
		    utf8_p = bytes_to_utf8(p, &len);
		}
		if (swash_fetch(sw, utf8_p, 1))
		    match = TRUE;
		else if (flags & ANYOF_FOLD) {
		    if (!match && lenp && av) {
		        I32 i;
			for (i = 0; i <= av_len(av); i++) {
			    SV* const sv = *av_fetch(av, i, FALSE);
			    STRLEN len;
			    const char * const s = SvPV_const(sv, len);
			    if (len <= plen && memEQ(s, (char*)utf8_p, len)) {
			        *lenp = len;
				match = TRUE;
				break;
			    }
			}
		    }
		    if (!match) {
		        U8 tmpbuf[UTF8_MAXBYTES_CASE+1];

			STRLEN tmplen;
			to_utf8_fold(utf8_p, tmpbuf, &tmplen);
			if (swash_fetch(sw, tmpbuf, 1))
			    match = TRUE;
		    }
		}

		/* If we allocated a string above, free it */
		if (! do_utf8) Safefree(utf8_p);
	    }
	}
	if (match && lenp && *lenp == 0)
	    *lenp = UNISKIP(NATIVE_TO_UNI(c));
    }
    if (!match && c < 256) {
	if (ANYOF_BITMAP_TEST(n, c))
	    match = TRUE;
	else if (flags & ANYOF_FOLD) {
	    U8 f;

	    if (flags & ANYOF_LOCALE) {
		PL_reg_flags |= RF_tainted;
		f = PL_fold_locale[c];
	    }
	    else
		f = PL_fold[c];
	    if (f != c && ANYOF_BITMAP_TEST(n, f))
		match = TRUE;
	}
	
	if (!match && (flags & ANYOF_CLASS)) {
	    PL_reg_flags |= RF_tainted;
	    if (
		(ANYOF_CLASS_TEST(n, ANYOF_ALNUM)   &&  isALNUM_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NALNUM)  && !isALNUM_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_SPACE)   &&  isSPACE_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NSPACE)  && !isSPACE_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_DIGIT)   &&  isDIGIT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NDIGIT)  && !isDIGIT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_ALNUMC)  &&  isALNUMC_LC(c)) ||
		(ANYOF_CLASS_TEST(n, ANYOF_NALNUMC) && !isALNUMC_LC(c)) ||
		(ANYOF_CLASS_TEST(n, ANYOF_ALPHA)   &&  isALPHA_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NALPHA)  && !isALPHA_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_ASCII)   &&  isASCII(c))     ||
		(ANYOF_CLASS_TEST(n, ANYOF_NASCII)  && !isASCII(c))     ||
		(ANYOF_CLASS_TEST(n, ANYOF_CNTRL)   &&  isCNTRL_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NCNTRL)  && !isCNTRL_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_GRAPH)   &&  isGRAPH_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NGRAPH)  && !isGRAPH_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_LOWER)   &&  isLOWER_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NLOWER)  && !isLOWER_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_PRINT)   &&  isPRINT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NPRINT)  && !isPRINT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_PUNCT)   &&  isPUNCT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NPUNCT)  && !isPUNCT_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_UPPER)   &&  isUPPER_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_NUPPER)  && !isUPPER_LC(c))  ||
		(ANYOF_CLASS_TEST(n, ANYOF_XDIGIT)  &&  isXDIGIT(c))    ||
		(ANYOF_CLASS_TEST(n, ANYOF_NXDIGIT) && !isXDIGIT(c))    ||
		(ANYOF_CLASS_TEST(n, ANYOF_PSXSPC)  &&  isPSXSPC(c))    ||
		(ANYOF_CLASS_TEST(n, ANYOF_NPSXSPC) && !isPSXSPC(c))    ||
		(ANYOF_CLASS_TEST(n, ANYOF_BLANK)   &&  isBLANK(c))     ||
		(ANYOF_CLASS_TEST(n, ANYOF_NBLANK)  && !isBLANK(c))
		) /* How's that for a conditional? */
	    {
		match = TRUE;
	    }
	}
    }

    return (flags & ANYOF_INVERT) ? !match : match;
}

STATIC U8 *
S_reghop3(U8 *s, I32 off, const U8* lim)
{
    dVAR;

    PERL_ARGS_ASSERT_REGHOP3;

    if (off >= 0) {
	while (off-- && s < lim) {
	    /* XXX could check well-formedness here */
	    s += UTF8SKIP(s);
	}
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
	    }
            /* XXX could check well-formedness here */
	}
    }
    return s;
}

#ifdef XXX_dmq
/* there are a bunch of places where we use two reghop3's that should
   be replaced with this routine. but since thats not done yet 
   we ifdef it out - dmq
*/
STATIC U8 *
S_reghop4(U8 *s, I32 off, const U8* llim, const U8* rlim)
{
    dVAR;

    PERL_ARGS_ASSERT_REGHOP4;

    if (off >= 0) {
        while (off-- && s < rlim) {
            /* XXX could check well-formedness here */
            s += UTF8SKIP(s);
        }
    }
    else {
        while (off++ && s > llim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > llim && UTF8_IS_CONTINUATION(*s))
                    s--;
            }
            /* XXX could check well-formedness here */
        }
    }
    return s;
}
#endif

STATIC U8 *
S_reghopmaybe3(U8* s, I32 off, const U8* lim)
{
    dVAR;

    PERL_ARGS_ASSERT_REGHOPMAYBE3;

    if (off >= 0) {
	while (off-- && s < lim) {
	    /* XXX could check well-formedness here */
	    s += UTF8SKIP(s);
	}
	if (off >= 0)
	    return NULL;
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
	    }
            /* XXX could check well-formedness here */
	}
	if (off <= 0)
	    return NULL;
    }
    return s;
}

static void
restore_pos(pTHX_ void *arg)
{
    dVAR;
    regexp * const rex = (regexp *)arg;
    if (PL_reg_eval_set) {
	if (PL_reg_oldsaved) {
	    rex->subbeg = PL_reg_oldsaved;
	    rex->sublen = PL_reg_oldsavedlen;
#ifdef PERL_OLD_COPY_ON_WRITE
	    rex->saved_copy = PL_nrs;
#endif
	    RXp_MATCH_COPIED_on(rex);
	}
	PL_reg_magic->mg_len = PL_reg_oldpos;
	PL_reg_eval_set = 0;
	PL_curpm = PL_reg_oldcurpm;
    }	
}

STATIC void
S_to_utf8_substr(pTHX_ register regexp *prog)
{
    int i = 1;

    PERL_ARGS_ASSERT_TO_UTF8_SUBSTR;

    do {
	if (prog->substrs->data[i].substr
	    && !prog->substrs->data[i].utf8_substr) {
	    SV* const sv = newSVsv(prog->substrs->data[i].substr);
	    prog->substrs->data[i].utf8_substr = sv;
	    sv_utf8_upgrade(sv);
	    if (SvVALID(prog->substrs->data[i].substr)) {
		const U8 flags = BmFLAGS(prog->substrs->data[i].substr);
		if (flags & FBMcf_TAIL) {
		    /* Trim the trailing \n that fbm_compile added last
		       time.  */
		    SvCUR_set(sv, SvCUR(sv) - 1);
		    /* Whilst this makes the SV technically "invalid" (as its
		       buffer is no longer followed by "\0") when fbm_compile()
		       adds the "\n" back, a "\0" is restored.  */
		}
		fbm_compile(sv, flags);
	    }
	    if (prog->substrs->data[i].substr == prog->check_substr)
		prog->check_utf8 = sv;
	}
    } while (i--);
}

STATIC void
S_to_byte_substr(pTHX_ register regexp *prog)
{
    dVAR;
    int i = 1;

    PERL_ARGS_ASSERT_TO_BYTE_SUBSTR;

    do {
	if (prog->substrs->data[i].utf8_substr
	    && !prog->substrs->data[i].substr) {
	    SV* sv = newSVsv(prog->substrs->data[i].utf8_substr);
	    if (sv_utf8_downgrade(sv, TRUE)) {
		if (SvVALID(prog->substrs->data[i].utf8_substr)) {
		    const U8 flags
			= BmFLAGS(prog->substrs->data[i].utf8_substr);
		    if (flags & FBMcf_TAIL) {
			/* Trim the trailing \n that fbm_compile added last
			   time.  */
			SvCUR_set(sv, SvCUR(sv) - 1);
		    }
		    fbm_compile(sv, flags);
		}	    
	    } else {
		SvREFCNT_dec(sv);
		sv = &PL_sv_undef;
	    }
	    prog->substrs->data[i].substr = sv;
	    if (prog->substrs->data[i].utf8_substr == prog->check_utf8)
		prog->check_substr = sv;
	}
    } while (i--);
}

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
