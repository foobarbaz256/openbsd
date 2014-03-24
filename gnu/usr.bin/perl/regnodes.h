/* -*- buffer-read-only: t -*-
   !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
   This file is built by regen/regcomp.pl from regcomp.sym.
   Any changes made here will be lost!
 */

/* Regops and State definitions */

#define REGNODE_MAX           	95
#define REGMATCH_STATE_MAX    	135

#define	END                   	0	/* 0000 End of program. */
#define	SUCCEED               	1	/* 0x01 Return from a subroutine, basically. */
#define	BOL                   	2	/* 0x02 Match "" at beginning of line. */
#define	MBOL                  	3	/* 0x03 Same, assuming multiline. */
#define	SBOL                  	4	/* 0x04 Same, assuming singleline. */
#define	EOS                   	5	/* 0x05 Match "" at end of string. */
#define	EOL                   	6	/* 0x06 Match "" at end of line. */
#define	MEOL                  	7	/* 0x07 Same, assuming multiline. */
#define	SEOL                  	8	/* 0x08 Same, assuming singleline. */
#define	BOUND                 	9	/* 0x09 Match "" at any word boundary using native charset semantics for non-utf8 */
#define	BOUNDL                	10	/* 0x0a Match "" at any locale word boundary */
#define	BOUNDU                	11	/* 0x0b Match "" at any word boundary using Unicode semantics */
#define	BOUNDA                	12	/* 0x0c Match "" at any word boundary using ASCII semantics */
#define	NBOUND                	13	/* 0x0d Match "" at any word non-boundary using native charset semantics for non-utf8 */
#define	NBOUNDL               	14	/* 0x0e Match "" at any locale word non-boundary */
#define	NBOUNDU               	15	/* 0x0f Match "" at any word non-boundary using Unicode semantics */
#define	NBOUNDA               	16	/* 0x10 Match "" at any word non-boundary using ASCII semantics */
#define	GPOS                  	17	/* 0x11 Matches where last m//g left off. */
#define	REG_ANY               	18	/* 0x12 Match any one character (except newline). */
#define	SANY                  	19	/* 0x13 Match any one character. */
#define	CANY                  	20	/* 0x14 Match any one byte. */
#define	ANYOF                 	21	/* 0x15 Match character in (or not in) this class, single char match only */
#define	ANYOF_WARN_SUPER      	22	/* 0x16 Match character in (or not in) this class, warn (if enabled) upon matching a char above Unicode max; */
#define	ANYOF_SYNTHETIC       	23	/* 0x17 Synthetic start class */
#define	POSIXD                	24	/* 0x18 Some [[:class:]] under /d; the FLAGS field gives which one */
#define	POSIXL                	25	/* 0x19 Some [[:class:]] under /l; the FLAGS field gives which one */
#define	POSIXU                	26	/* 0x1a Some [[:class:]] under /u; the FLAGS field gives which one */
#define	POSIXA                	27	/* 0x1b Some [[:class:]] under /a; the FLAGS field gives which one */
#define	NPOSIXD               	28	/* 0x1c complement of POSIXD, [[:^class:]] */
#define	NPOSIXL               	29	/* 0x1d complement of POSIXL, [[:^class:]] */
#define	NPOSIXU               	30	/* 0x1e complement of POSIXU, [[:^class:]] */
#define	NPOSIXA               	31	/* 0x1f complement of POSIXA, [[:^class:]] */
#define	CLUMP                 	32	/* 0x20 Match any extended grapheme cluster sequence */
#define	BRANCH                	33	/* 0x21 Match this alternative, or the next... */
#define	BACK                  	34	/* 0x22 Match "", "next" ptr points backward. */
#define	EXACT                 	35	/* 0x23 Match this string (preceded by length). */
#define	EXACTF                	36	/* 0x24 Match this non-UTF-8 string (not guaranteed to be folded) using /id rules (w/len). */
#define	EXACTFL               	37	/* 0x25 Match this string (not guaranteed to be folded) using /il rules (w/len). */
#define	EXACTFU               	38	/* 0x26 Match this string (folded iff in UTF-8, length in folding doesn't change if not in UTF-8) using /iu rules (w/len). */
#define	EXACTFA               	39	/* 0x27 Match this string (not guaranteed to be folded) using /iaa rules (w/len). */
#define	EXACTFU_SS            	40	/* 0x28 Match this string (folded iff in UTF-8, length in folding may change even if not in UTF-8) using /iu rules (w/len). */
#define	EXACTFU_TRICKYFOLD    	41	/* 0x29 Match this folded UTF-8 string using /iu rules */
#define	NOTHING               	42	/* 0x2a Match empty string. */
#define	TAIL                  	43	/* 0x2b Match empty string. Can jump here from outside. */
#define	STAR                  	44	/* 0x2c Match this (simple) thing 0 or more times. */
#define	PLUS                  	45	/* 0x2d Match this (simple) thing 1 or more times. */
#define	CURLY                 	46	/* 0x2e Match this simple thing {n,m} times. */
#define	CURLYN                	47	/* 0x2f Capture next-after-this simple thing */
#define	CURLYM                	48	/* 0x30 Capture this medium-complex thing {n,m} times. */
#define	CURLYX                	49	/* 0x31 Match this complex thing {n,m} times. */
#define	WHILEM                	50	/* 0x32 Do curly processing and see if rest matches. */
#define	OPEN                  	51	/* 0x33 Mark this point in input as start of #n. */
#define	CLOSE                 	52	/* 0x34 Analogous to OPEN. */
#define	REF                   	53	/* 0x35 Match some already matched string */
#define	REFF                  	54	/* 0x36 Match already matched string, folded using native charset semantics for non-utf8 */
#define	REFFL                 	55	/* 0x37 Match already matched string, folded in loc. */
#define	REFFU                 	56	/* 0x38 Match already matched string, folded using unicode semantics for non-utf8 */
#define	REFFA                 	57	/* 0x39 Match already matched string, folded using unicode semantics for non-utf8, no mixing ASCII, non-ASCII */
#define	NREF                  	58	/* 0x3a Match some already matched string */
#define	NREFF                 	59	/* 0x3b Match already matched string, folded using native charset semantics for non-utf8 */
#define	NREFFL                	60	/* 0x3c Match already matched string, folded in loc. */
#define	NREFFU                	61	/* 0x3d Match already matched string, folded using unicode semantics for non-utf8 */
#define	NREFFA                	62	/* 0x3e Match already matched string, folded using unicode semantics for non-utf8, no mixing ASCII, non-ASCII */
#define	IFMATCH               	63	/* 0x3f Succeeds if the following matches. */
#define	UNLESSM               	64	/* 0x40 Fails if the following matches. */
#define	SUSPEND               	65	/* 0x41 "Independent" sub-RE. */
#define	IFTHEN                	66	/* 0x42 Switch, should be preceded by switcher. */
#define	GROUPP                	67	/* 0x43 Whether the group matched. */
#define	LONGJMP               	68	/* 0x44 Jump far away. */
#define	BRANCHJ               	69	/* 0x45 BRANCH with long offset. */
#define	EVAL                  	70	/* 0x46 Execute some Perl code. */
#define	MINMOD                	71	/* 0x47 Next operator is not greedy. */
#define	LOGICAL               	72	/* 0x48 Next opcode should set the flag only. */
#define	RENUM                 	73	/* 0x49 Group with independently numbered parens. */
#define	TRIE                  	74	/* 0x4a Match many EXACT(F[ALU]?)? at once. flags==type */
#define	TRIEC                 	75	/* 0x4b Same as TRIE, but with embedded charclass data */
#define	AHOCORASICK           	76	/* 0x4c Aho Corasick stclass. flags==type */
#define	AHOCORASICKC          	77	/* 0x4d Same as AHOCORASICK, but with embedded charclass data */
#define	GOSUB                 	78	/* 0x4e recurse to paren arg1 at (signed) ofs arg2 */
#define	GOSTART               	79	/* 0x4f recurse to start of pattern */
#define	NGROUPP               	80	/* 0x50 Whether the group matched. */
#define	INSUBP                	81	/* 0x51 Whether we are in a specific recurse. */
#define	DEFINEP               	82	/* 0x52 Never execute directly. */
#define	ENDLIKE               	83	/* 0x53 Used only for the type field of verbs */
#define	OPFAIL                	84	/* 0x54 Same as (?!) */
#define	ACCEPT                	85	/* 0x55 Accepts the current matched string. */
#define	VERB                  	86	/* 0x56 Used only for the type field of verbs */
#define	PRUNE                 	87	/* 0x57 Pattern fails at this startpoint if no-backtracking through this */
#define	MARKPOINT             	88	/* 0x58 Push the current location for rollback by cut. */
#define	SKIP                  	89	/* 0x59 On failure skip forward (to the mark) before retrying */
#define	COMMIT                	90	/* 0x5a Pattern fails outright if backtracking through this */
#define	CUTGROUP              	91	/* 0x5b On failure go to the next alternation in the group */
#define	KEEPS                 	92	/* 0x5c $& begins here. */
#define	LNBREAK               	93	/* 0x5d generic newline pattern */
#define	OPTIMIZED             	94	/* 0x5e Placeholder for dump. */
#define	PSEUDO                	95	/* 0x5f Pseudo opcode for internal use. */
	/* ------------ States ------------- */
#define	TRIE_next             	(REGNODE_MAX + 1)	/* state for TRIE */
#define	TRIE_next_fail        	(REGNODE_MAX + 2)	/* state for TRIE */
#define	EVAL_AB               	(REGNODE_MAX + 3)	/* state for EVAL */
#define	EVAL_AB_fail          	(REGNODE_MAX + 4)	/* state for EVAL */
#define	CURLYX_end            	(REGNODE_MAX + 5)	/* state for CURLYX */
#define	CURLYX_end_fail       	(REGNODE_MAX + 6)	/* state for CURLYX */
#define	WHILEM_A_pre          	(REGNODE_MAX + 7)	/* state for WHILEM */
#define	WHILEM_A_pre_fail     	(REGNODE_MAX + 8)	/* state for WHILEM */
#define	WHILEM_A_min          	(REGNODE_MAX + 9)	/* state for WHILEM */
#define	WHILEM_A_min_fail     	(REGNODE_MAX + 10)	/* state for WHILEM */
#define	WHILEM_A_max          	(REGNODE_MAX + 11)	/* state for WHILEM */
#define	WHILEM_A_max_fail     	(REGNODE_MAX + 12)	/* state for WHILEM */
#define	WHILEM_B_min          	(REGNODE_MAX + 13)	/* state for WHILEM */
#define	WHILEM_B_min_fail     	(REGNODE_MAX + 14)	/* state for WHILEM */
#define	WHILEM_B_max          	(REGNODE_MAX + 15)	/* state for WHILEM */
#define	WHILEM_B_max_fail     	(REGNODE_MAX + 16)	/* state for WHILEM */
#define	BRANCH_next           	(REGNODE_MAX + 17)	/* state for BRANCH */
#define	BRANCH_next_fail      	(REGNODE_MAX + 18)	/* state for BRANCH */
#define	CURLYM_A              	(REGNODE_MAX + 19)	/* state for CURLYM */
#define	CURLYM_A_fail         	(REGNODE_MAX + 20)	/* state for CURLYM */
#define	CURLYM_B              	(REGNODE_MAX + 21)	/* state for CURLYM */
#define	CURLYM_B_fail         	(REGNODE_MAX + 22)	/* state for CURLYM */
#define	IFMATCH_A             	(REGNODE_MAX + 23)	/* state for IFMATCH */
#define	IFMATCH_A_fail        	(REGNODE_MAX + 24)	/* state for IFMATCH */
#define	CURLY_B_min_known     	(REGNODE_MAX + 25)	/* state for CURLY */
#define	CURLY_B_min_known_fail	(REGNODE_MAX + 26)	/* state for CURLY */
#define	CURLY_B_min           	(REGNODE_MAX + 27)	/* state for CURLY */
#define	CURLY_B_min_fail      	(REGNODE_MAX + 28)	/* state for CURLY */
#define	CURLY_B_max           	(REGNODE_MAX + 29)	/* state for CURLY */
#define	CURLY_B_max_fail      	(REGNODE_MAX + 30)	/* state for CURLY */
#define	COMMIT_next           	(REGNODE_MAX + 31)	/* state for COMMIT */
#define	COMMIT_next_fail      	(REGNODE_MAX + 32)	/* state for COMMIT */
#define	MARKPOINT_next        	(REGNODE_MAX + 33)	/* state for MARKPOINT */
#define	MARKPOINT_next_fail   	(REGNODE_MAX + 34)	/* state for MARKPOINT */
#define	SKIP_next             	(REGNODE_MAX + 35)	/* state for SKIP */
#define	SKIP_next_fail        	(REGNODE_MAX + 36)	/* state for SKIP */
#define	CUTGROUP_next         	(REGNODE_MAX + 37)	/* state for CUTGROUP */
#define	CUTGROUP_next_fail    	(REGNODE_MAX + 38)	/* state for CUTGROUP */
#define	KEEPS_next            	(REGNODE_MAX + 39)	/* state for KEEPS */
#define	KEEPS_next_fail       	(REGNODE_MAX + 40)	/* state for KEEPS */

/* PL_regkind[] What type of regop or state is this. */

#ifndef DOINIT
EXTCONST U8 PL_regkind[];
#else
EXTCONST U8 PL_regkind[] = {
	END,      	/* END                    */
	END,      	/* SUCCEED                */
	BOL,      	/* BOL                    */
	BOL,      	/* MBOL                   */
	BOL,      	/* SBOL                   */
	EOL,      	/* EOS                    */
	EOL,      	/* EOL                    */
	EOL,      	/* MEOL                   */
	EOL,      	/* SEOL                   */
	BOUND,    	/* BOUND                  */
	BOUND,    	/* BOUNDL                 */
	BOUND,    	/* BOUNDU                 */
	BOUND,    	/* BOUNDA                 */
	NBOUND,   	/* NBOUND                 */
	NBOUND,   	/* NBOUNDL                */
	NBOUND,   	/* NBOUNDU                */
	NBOUND,   	/* NBOUNDA                */
	GPOS,     	/* GPOS                   */
	REG_ANY,  	/* REG_ANY                */
	REG_ANY,  	/* SANY                   */
	REG_ANY,  	/* CANY                   */
	ANYOF,    	/* ANYOF                  */
	ANYOF,    	/* ANYOF_WARN_SUPER       */
	ANYOF,    	/* ANYOF_SYNTHETIC        */
	POSIXD,   	/* POSIXD                 */
	POSIXD,   	/* POSIXL                 */
	POSIXD,   	/* POSIXU                 */
	POSIXD,   	/* POSIXA                 */
	NPOSIXD,  	/* NPOSIXD                */
	NPOSIXD,  	/* NPOSIXL                */
	NPOSIXD,  	/* NPOSIXU                */
	NPOSIXD,  	/* NPOSIXA                */
	CLUMP,    	/* CLUMP                  */
	BRANCH,   	/* BRANCH                 */
	BACK,     	/* BACK                   */
	EXACT,    	/* EXACT                  */
	EXACT,    	/* EXACTF                 */
	EXACT,    	/* EXACTFL                */
	EXACT,    	/* EXACTFU                */
	EXACT,    	/* EXACTFA                */
	EXACT,    	/* EXACTFU_SS             */
	EXACT,    	/* EXACTFU_TRICKYFOLD     */
	NOTHING,  	/* NOTHING                */
	NOTHING,  	/* TAIL                   */
	STAR,     	/* STAR                   */
	PLUS,     	/* PLUS                   */
	CURLY,    	/* CURLY                  */
	CURLY,    	/* CURLYN                 */
	CURLY,    	/* CURLYM                 */
	CURLY,    	/* CURLYX                 */
	WHILEM,   	/* WHILEM                 */
	OPEN,     	/* OPEN                   */
	CLOSE,    	/* CLOSE                  */
	REF,      	/* REF                    */
	REF,      	/* REFF                   */
	REF,      	/* REFFL                  */
	REF,      	/* REFFU                  */
	REF,      	/* REFFA                  */
	REF,      	/* NREF                   */
	REF,      	/* NREFF                  */
	REF,      	/* NREFFL                 */
	REF,      	/* NREFFU                 */
	REF,      	/* NREFFA                 */
	BRANCHJ,  	/* IFMATCH                */
	BRANCHJ,  	/* UNLESSM                */
	BRANCHJ,  	/* SUSPEND                */
	BRANCHJ,  	/* IFTHEN                 */
	GROUPP,   	/* GROUPP                 */
	LONGJMP,  	/* LONGJMP                */
	BRANCHJ,  	/* BRANCHJ                */
	EVAL,     	/* EVAL                   */
	MINMOD,   	/* MINMOD                 */
	LOGICAL,  	/* LOGICAL                */
	BRANCHJ,  	/* RENUM                  */
	TRIE,     	/* TRIE                   */
	TRIE,     	/* TRIEC                  */
	TRIE,     	/* AHOCORASICK            */
	TRIE,     	/* AHOCORASICKC           */
	GOSUB,    	/* GOSUB                  */
	GOSTART,  	/* GOSTART                */
	NGROUPP,  	/* NGROUPP                */
	INSUBP,   	/* INSUBP                 */
	DEFINEP,  	/* DEFINEP                */
	ENDLIKE,  	/* ENDLIKE                */
	ENDLIKE,  	/* OPFAIL                 */
	ENDLIKE,  	/* ACCEPT                 */
	VERB,     	/* VERB                   */
	VERB,     	/* PRUNE                  */
	VERB,     	/* MARKPOINT              */
	VERB,     	/* SKIP                   */
	VERB,     	/* COMMIT                 */
	VERB,     	/* CUTGROUP               */
	KEEPS,    	/* KEEPS                  */
	LNBREAK,  	/* LNBREAK                */
	NOTHING,  	/* OPTIMIZED              */
	PSEUDO,   	/* PSEUDO                 */
	/* ------------ States ------------- */
	TRIE,     	/* TRIE_next              */
	TRIE,     	/* TRIE_next_fail         */
	EVAL,     	/* EVAL_AB                */
	EVAL,     	/* EVAL_AB_fail           */
	CURLYX,   	/* CURLYX_end             */
	CURLYX,   	/* CURLYX_end_fail        */
	WHILEM,   	/* WHILEM_A_pre           */
	WHILEM,   	/* WHILEM_A_pre_fail      */
	WHILEM,   	/* WHILEM_A_min           */
	WHILEM,   	/* WHILEM_A_min_fail      */
	WHILEM,   	/* WHILEM_A_max           */
	WHILEM,   	/* WHILEM_A_max_fail      */
	WHILEM,   	/* WHILEM_B_min           */
	WHILEM,   	/* WHILEM_B_min_fail      */
	WHILEM,   	/* WHILEM_B_max           */
	WHILEM,   	/* WHILEM_B_max_fail      */
	BRANCH,   	/* BRANCH_next            */
	BRANCH,   	/* BRANCH_next_fail       */
	CURLYM,   	/* CURLYM_A               */
	CURLYM,   	/* CURLYM_A_fail          */
	CURLYM,   	/* CURLYM_B               */
	CURLYM,   	/* CURLYM_B_fail          */
	IFMATCH,  	/* IFMATCH_A              */
	IFMATCH,  	/* IFMATCH_A_fail         */
	CURLY,    	/* CURLY_B_min_known      */
	CURLY,    	/* CURLY_B_min_known_fail */
	CURLY,    	/* CURLY_B_min            */
	CURLY,    	/* CURLY_B_min_fail       */
	CURLY,    	/* CURLY_B_max            */
	CURLY,    	/* CURLY_B_max_fail       */
	COMMIT,   	/* COMMIT_next            */
	COMMIT,   	/* COMMIT_next_fail       */
	MARKPOINT,	/* MARKPOINT_next         */
	MARKPOINT,	/* MARKPOINT_next_fail    */
	SKIP,     	/* SKIP_next              */
	SKIP,     	/* SKIP_next_fail         */
	CUTGROUP, 	/* CUTGROUP_next          */
	CUTGROUP, 	/* CUTGROUP_next_fail     */
	KEEPS,    	/* KEEPS_next             */
	KEEPS,    	/* KEEPS_next_fail        */
};
#endif

/* regarglen[] - How large is the argument part of the node (in regnodes) */

#ifdef REG_COMP_C
static const U8 regarglen[] = {
	0,                                   	/* END          */
	0,                                   	/* SUCCEED      */
	0,                                   	/* BOL          */
	0,                                   	/* MBOL         */
	0,                                   	/* SBOL         */
	0,                                   	/* EOS          */
	0,                                   	/* EOL          */
	0,                                   	/* MEOL         */
	0,                                   	/* SEOL         */
	0,                                   	/* BOUND        */
	0,                                   	/* BOUNDL       */
	0,                                   	/* BOUNDU       */
	0,                                   	/* BOUNDA       */
	0,                                   	/* NBOUND       */
	0,                                   	/* NBOUNDL      */
	0,                                   	/* NBOUNDU      */
	0,                                   	/* NBOUNDA      */
	0,                                   	/* GPOS         */
	0,                                   	/* REG_ANY      */
	0,                                   	/* SANY         */
	0,                                   	/* CANY         */
	0,                                   	/* ANYOF        */
	0,                                   	/* ANYOF_WARN_SUPER */
	0,                                   	/* ANYOF_SYNTHETIC */
	0,                                   	/* POSIXD       */
	0,                                   	/* POSIXL       */
	0,                                   	/* POSIXU       */
	0,                                   	/* POSIXA       */
	0,                                   	/* NPOSIXD      */
	0,                                   	/* NPOSIXL      */
	0,                                   	/* NPOSIXU      */
	0,                                   	/* NPOSIXA      */
	0,                                   	/* CLUMP        */
	0,                                   	/* BRANCH       */
	0,                                   	/* BACK         */
	0,                                   	/* EXACT        */
	0,                                   	/* EXACTF       */
	0,                                   	/* EXACTFL      */
	0,                                   	/* EXACTFU      */
	0,                                   	/* EXACTFA      */
	0,                                   	/* EXACTFU_SS   */
	0,                                   	/* EXACTFU_TRICKYFOLD */
	0,                                   	/* NOTHING      */
	0,                                   	/* TAIL         */
	0,                                   	/* STAR         */
	0,                                   	/* PLUS         */
	EXTRA_SIZE(struct regnode_2),        	/* CURLY        */
	EXTRA_SIZE(struct regnode_2),        	/* CURLYN       */
	EXTRA_SIZE(struct regnode_2),        	/* CURLYM       */
	EXTRA_SIZE(struct regnode_2),        	/* CURLYX       */
	0,                                   	/* WHILEM       */
	EXTRA_SIZE(struct regnode_1),        	/* OPEN         */
	EXTRA_SIZE(struct regnode_1),        	/* CLOSE        */
	EXTRA_SIZE(struct regnode_1),        	/* REF          */
	EXTRA_SIZE(struct regnode_1),        	/* REFF         */
	EXTRA_SIZE(struct regnode_1),        	/* REFFL        */
	EXTRA_SIZE(struct regnode_1),        	/* REFFU        */
	EXTRA_SIZE(struct regnode_1),        	/* REFFA        */
	EXTRA_SIZE(struct regnode_1),        	/* NREF         */
	EXTRA_SIZE(struct regnode_1),        	/* NREFF        */
	EXTRA_SIZE(struct regnode_1),        	/* NREFFL       */
	EXTRA_SIZE(struct regnode_1),        	/* NREFFU       */
	EXTRA_SIZE(struct regnode_1),        	/* NREFFA       */
	EXTRA_SIZE(struct regnode_1),        	/* IFMATCH      */
	EXTRA_SIZE(struct regnode_1),        	/* UNLESSM      */
	EXTRA_SIZE(struct regnode_1),        	/* SUSPEND      */
	EXTRA_SIZE(struct regnode_1),        	/* IFTHEN       */
	EXTRA_SIZE(struct regnode_1),        	/* GROUPP       */
	EXTRA_SIZE(struct regnode_1),        	/* LONGJMP      */
	EXTRA_SIZE(struct regnode_1),        	/* BRANCHJ      */
	EXTRA_SIZE(struct regnode_1),        	/* EVAL         */
	0,                                   	/* MINMOD       */
	0,                                   	/* LOGICAL      */
	EXTRA_SIZE(struct regnode_1),        	/* RENUM        */
	EXTRA_SIZE(struct regnode_1),        	/* TRIE         */
	EXTRA_SIZE(struct regnode_charclass),	/* TRIEC        */
	EXTRA_SIZE(struct regnode_1),        	/* AHOCORASICK  */
	EXTRA_SIZE(struct regnode_charclass),	/* AHOCORASICKC */
	EXTRA_SIZE(struct regnode_2L),       	/* GOSUB        */
	0,                                   	/* GOSTART      */
	EXTRA_SIZE(struct regnode_1),        	/* NGROUPP      */
	EXTRA_SIZE(struct regnode_1),        	/* INSUBP       */
	EXTRA_SIZE(struct regnode_1),        	/* DEFINEP      */
	0,                                   	/* ENDLIKE      */
	0,                                   	/* OPFAIL       */
	EXTRA_SIZE(struct regnode_1),        	/* ACCEPT       */
	EXTRA_SIZE(struct regnode_1),        	/* VERB         */
	EXTRA_SIZE(struct regnode_1),        	/* PRUNE        */
	EXTRA_SIZE(struct regnode_1),        	/* MARKPOINT    */
	EXTRA_SIZE(struct regnode_1),        	/* SKIP         */
	EXTRA_SIZE(struct regnode_1),        	/* COMMIT       */
	EXTRA_SIZE(struct regnode_1),        	/* CUTGROUP     */
	0,                                   	/* KEEPS        */
	0,                                   	/* LNBREAK      */
	0,                                   	/* OPTIMIZED    */
	0,                                   	/* PSEUDO       */
};

/* reg_off_by_arg[] - Which argument holds the offset to the next node */

static const char reg_off_by_arg[] = {
	0,	/* END          */
	0,	/* SUCCEED      */
	0,	/* BOL          */
	0,	/* MBOL         */
	0,	/* SBOL         */
	0,	/* EOS          */
	0,	/* EOL          */
	0,	/* MEOL         */
	0,	/* SEOL         */
	0,	/* BOUND        */
	0,	/* BOUNDL       */
	0,	/* BOUNDU       */
	0,	/* BOUNDA       */
	0,	/* NBOUND       */
	0,	/* NBOUNDL      */
	0,	/* NBOUNDU      */
	0,	/* NBOUNDA      */
	0,	/* GPOS         */
	0,	/* REG_ANY      */
	0,	/* SANY         */
	0,	/* CANY         */
	0,	/* ANYOF        */
	0,	/* ANYOF_WARN_SUPER */
	0,	/* ANYOF_SYNTHETIC */
	0,	/* POSIXD       */
	0,	/* POSIXL       */
	0,	/* POSIXU       */
	0,	/* POSIXA       */
	0,	/* NPOSIXD      */
	0,	/* NPOSIXL      */
	0,	/* NPOSIXU      */
	0,	/* NPOSIXA      */
	0,	/* CLUMP        */
	0,	/* BRANCH       */
	0,	/* BACK         */
	0,	/* EXACT        */
	0,	/* EXACTF       */
	0,	/* EXACTFL      */
	0,	/* EXACTFU      */
	0,	/* EXACTFA      */
	0,	/* EXACTFU_SS   */
	0,	/* EXACTFU_TRICKYFOLD */
	0,	/* NOTHING      */
	0,	/* TAIL         */
	0,	/* STAR         */
	0,	/* PLUS         */
	0,	/* CURLY        */
	0,	/* CURLYN       */
	0,	/* CURLYM       */
	0,	/* CURLYX       */
	0,	/* WHILEM       */
	0,	/* OPEN         */
	0,	/* CLOSE        */
	0,	/* REF          */
	0,	/* REFF         */
	0,	/* REFFL        */
	0,	/* REFFU        */
	0,	/* REFFA        */
	0,	/* NREF         */
	0,	/* NREFF        */
	0,	/* NREFFL       */
	0,	/* NREFFU       */
	0,	/* NREFFA       */
	2,	/* IFMATCH      */
	2,	/* UNLESSM      */
	1,	/* SUSPEND      */
	1,	/* IFTHEN       */
	0,	/* GROUPP       */
	1,	/* LONGJMP      */
	1,	/* BRANCHJ      */
	0,	/* EVAL         */
	0,	/* MINMOD       */
	0,	/* LOGICAL      */
	1,	/* RENUM        */
	0,	/* TRIE         */
	0,	/* TRIEC        */
	0,	/* AHOCORASICK  */
	0,	/* AHOCORASICKC */
	0,	/* GOSUB        */
	0,	/* GOSTART      */
	0,	/* NGROUPP      */
	0,	/* INSUBP       */
	0,	/* DEFINEP      */
	0,	/* ENDLIKE      */
	0,	/* OPFAIL       */
	0,	/* ACCEPT       */
	0,	/* VERB         */
	0,	/* PRUNE        */
	0,	/* MARKPOINT    */
	0,	/* SKIP         */
	0,	/* COMMIT       */
	0,	/* CUTGROUP     */
	0,	/* KEEPS        */
	0,	/* LNBREAK      */
	0,	/* OPTIMIZED    */
	0,	/* PSEUDO       */
};

#endif /* REG_COMP_C */

/* reg_name[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST char * PL_reg_name[];
#else
EXTCONST char * const PL_reg_name[] = {
	"END",                   	/* 0000 */
	"SUCCEED",               	/* 0x01 */
	"BOL",                   	/* 0x02 */
	"MBOL",                  	/* 0x03 */
	"SBOL",                  	/* 0x04 */
	"EOS",                   	/* 0x05 */
	"EOL",                   	/* 0x06 */
	"MEOL",                  	/* 0x07 */
	"SEOL",                  	/* 0x08 */
	"BOUND",                 	/* 0x09 */
	"BOUNDL",                	/* 0x0a */
	"BOUNDU",                	/* 0x0b */
	"BOUNDA",                	/* 0x0c */
	"NBOUND",                	/* 0x0d */
	"NBOUNDL",               	/* 0x0e */
	"NBOUNDU",               	/* 0x0f */
	"NBOUNDA",               	/* 0x10 */
	"GPOS",                  	/* 0x11 */
	"REG_ANY",               	/* 0x12 */
	"SANY",                  	/* 0x13 */
	"CANY",                  	/* 0x14 */
	"ANYOF",                 	/* 0x15 */
	"ANYOF_WARN_SUPER",      	/* 0x16 */
	"ANYOF_SYNTHETIC",       	/* 0x17 */
	"POSIXD",                	/* 0x18 */
	"POSIXL",                	/* 0x19 */
	"POSIXU",                	/* 0x1a */
	"POSIXA",                	/* 0x1b */
	"NPOSIXD",               	/* 0x1c */
	"NPOSIXL",               	/* 0x1d */
	"NPOSIXU",               	/* 0x1e */
	"NPOSIXA",               	/* 0x1f */
	"CLUMP",                 	/* 0x20 */
	"BRANCH",                	/* 0x21 */
	"BACK",                  	/* 0x22 */
	"EXACT",                 	/* 0x23 */
	"EXACTF",                	/* 0x24 */
	"EXACTFL",               	/* 0x25 */
	"EXACTFU",               	/* 0x26 */
	"EXACTFA",               	/* 0x27 */
	"EXACTFU_SS",            	/* 0x28 */
	"EXACTFU_TRICKYFOLD",    	/* 0x29 */
	"NOTHING",               	/* 0x2a */
	"TAIL",                  	/* 0x2b */
	"STAR",                  	/* 0x2c */
	"PLUS",                  	/* 0x2d */
	"CURLY",                 	/* 0x2e */
	"CURLYN",                	/* 0x2f */
	"CURLYM",                	/* 0x30 */
	"CURLYX",                	/* 0x31 */
	"WHILEM",                	/* 0x32 */
	"OPEN",                  	/* 0x33 */
	"CLOSE",                 	/* 0x34 */
	"REF",                   	/* 0x35 */
	"REFF",                  	/* 0x36 */
	"REFFL",                 	/* 0x37 */
	"REFFU",                 	/* 0x38 */
	"REFFA",                 	/* 0x39 */
	"NREF",                  	/* 0x3a */
	"NREFF",                 	/* 0x3b */
	"NREFFL",                	/* 0x3c */
	"NREFFU",                	/* 0x3d */
	"NREFFA",                	/* 0x3e */
	"IFMATCH",               	/* 0x3f */
	"UNLESSM",               	/* 0x40 */
	"SUSPEND",               	/* 0x41 */
	"IFTHEN",                	/* 0x42 */
	"GROUPP",                	/* 0x43 */
	"LONGJMP",               	/* 0x44 */
	"BRANCHJ",               	/* 0x45 */
	"EVAL",                  	/* 0x46 */
	"MINMOD",                	/* 0x47 */
	"LOGICAL",               	/* 0x48 */
	"RENUM",                 	/* 0x49 */
	"TRIE",                  	/* 0x4a */
	"TRIEC",                 	/* 0x4b */
	"AHOCORASICK",           	/* 0x4c */
	"AHOCORASICKC",          	/* 0x4d */
	"GOSUB",                 	/* 0x4e */
	"GOSTART",               	/* 0x4f */
	"NGROUPP",               	/* 0x50 */
	"INSUBP",                	/* 0x51 */
	"DEFINEP",               	/* 0x52 */
	"ENDLIKE",               	/* 0x53 */
	"OPFAIL",                	/* 0x54 */
	"ACCEPT",                	/* 0x55 */
	"VERB",                  	/* 0x56 */
	"PRUNE",                 	/* 0x57 */
	"MARKPOINT",             	/* 0x58 */
	"SKIP",                  	/* 0x59 */
	"COMMIT",                	/* 0x5a */
	"CUTGROUP",              	/* 0x5b */
	"KEEPS",                 	/* 0x5c */
	"LNBREAK",               	/* 0x5d */
	"OPTIMIZED",             	/* 0x5e */
	"PSEUDO",                	/* 0x5f */
	/* ------------ States ------------- */
	"TRIE_next",             	/* REGNODE_MAX +0x01 */
	"TRIE_next_fail",        	/* REGNODE_MAX +0x02 */
	"EVAL_AB",               	/* REGNODE_MAX +0x03 */
	"EVAL_AB_fail",          	/* REGNODE_MAX +0x04 */
	"CURLYX_end",            	/* REGNODE_MAX +0x05 */
	"CURLYX_end_fail",       	/* REGNODE_MAX +0x06 */
	"WHILEM_A_pre",          	/* REGNODE_MAX +0x07 */
	"WHILEM_A_pre_fail",     	/* REGNODE_MAX +0x08 */
	"WHILEM_A_min",          	/* REGNODE_MAX +0x09 */
	"WHILEM_A_min_fail",     	/* REGNODE_MAX +0x0a */
	"WHILEM_A_max",          	/* REGNODE_MAX +0x0b */
	"WHILEM_A_max_fail",     	/* REGNODE_MAX +0x0c */
	"WHILEM_B_min",          	/* REGNODE_MAX +0x0d */
	"WHILEM_B_min_fail",     	/* REGNODE_MAX +0x0e */
	"WHILEM_B_max",          	/* REGNODE_MAX +0x0f */
	"WHILEM_B_max_fail",     	/* REGNODE_MAX +0x10 */
	"BRANCH_next",           	/* REGNODE_MAX +0x11 */
	"BRANCH_next_fail",      	/* REGNODE_MAX +0x12 */
	"CURLYM_A",              	/* REGNODE_MAX +0x13 */
	"CURLYM_A_fail",         	/* REGNODE_MAX +0x14 */
	"CURLYM_B",              	/* REGNODE_MAX +0x15 */
	"CURLYM_B_fail",         	/* REGNODE_MAX +0x16 */
	"IFMATCH_A",             	/* REGNODE_MAX +0x17 */
	"IFMATCH_A_fail",        	/* REGNODE_MAX +0x18 */
	"CURLY_B_min_known",     	/* REGNODE_MAX +0x19 */
	"CURLY_B_min_known_fail",	/* REGNODE_MAX +0x1a */
	"CURLY_B_min",           	/* REGNODE_MAX +0x1b */
	"CURLY_B_min_fail",      	/* REGNODE_MAX +0x1c */
	"CURLY_B_max",           	/* REGNODE_MAX +0x1d */
	"CURLY_B_max_fail",      	/* REGNODE_MAX +0x1e */
	"COMMIT_next",           	/* REGNODE_MAX +0x1f */
	"COMMIT_next_fail",      	/* REGNODE_MAX +0x20 */
	"MARKPOINT_next",        	/* REGNODE_MAX +0x21 */
	"MARKPOINT_next_fail",   	/* REGNODE_MAX +0x22 */
	"SKIP_next",             	/* REGNODE_MAX +0x23 */
	"SKIP_next_fail",        	/* REGNODE_MAX +0x24 */
	"CUTGROUP_next",         	/* REGNODE_MAX +0x25 */
	"CUTGROUP_next_fail",    	/* REGNODE_MAX +0x26 */
	"KEEPS_next",            	/* REGNODE_MAX +0x27 */
	"KEEPS_next_fail",       	/* REGNODE_MAX +0x28 */
};
#endif /* DOINIT */

/* PL_reg_extflags_name[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST char * PL_reg_extflags_name[];
#else
EXTCONST char * const PL_reg_extflags_name[] = {
	/* Bits in extflags defined: 11111110111111111111111111111111 */
	"MULTILINE",        /* 0x00000001 */
	"SINGLELINE",       /* 0x00000002 */
	"FOLD",             /* 0x00000004 */
	"EXTENDED",         /* 0x00000008 */
	"KEEPCOPY",         /* 0x00000010 */
	"CHARSET0",         /* 0x00000020 : "CHARSET" - 0x000000e0 */
	"CHARSET1",         /* 0x00000040 : "CHARSET" - 0x000000e0 */
	"CHARSET2",         /* 0x00000080 : "CHARSET" - 0x000000e0 */
	"SPLIT",            /* 0x00000100 */
	"ANCH_BOL",         /* 0x00000200 */
	"ANCH_MBOL",        /* 0x00000400 */
	"ANCH_SBOL",        /* 0x00000800 */
	"ANCH_GPOS",        /* 0x00001000 */
	"GPOS_SEEN",        /* 0x00002000 */
	"GPOS_FLOAT",       /* 0x00004000 */
	"NO_INPLACE_SUBST", /* 0x00008000 */
	"EVAL_SEEN",        /* 0x00010000 */
	"CANY_SEEN",        /* 0x00020000 */
	"NOSCAN",           /* 0x00040000 */
	"CHECK_ALL",        /* 0x00080000 */
	"MATCH_UTF8",       /* 0x00100000 */
	"USE_INTUIT_NOML",  /* 0x00200000 */
	"USE_INTUIT_ML",    /* 0x00400000 */
	"INTUIT_TAIL",      /* 0x00800000 */
	"UNUSED_BIT_24",    /* 0x01000000 */
	"COPY_DONE",        /* 0x02000000 */
	"TAINTED_SEEN",     /* 0x04000000 */
	"TAINTED",          /* 0x08000000 */
	"START_ONLY",       /* 0x10000000 */
	"SKIPWHITE",        /* 0x20000000 */
	"WHITE",            /* 0x40000000 */
	"NULL",             /* 0x80000000 */
};
#endif /* DOINIT */

/* The following have no fixed length. U8 so we can do strchr() on it. */
#define REGNODE_VARIES(node) (PL_varies_bitmask[(node) >> 3] & (1 << ((node) & 7)))

#ifndef DOINIT
EXTCONST U8 PL_varies[] __attribute__deprecated__;
#else
EXTCONST U8 PL_varies[] __attribute__deprecated__ = {
    CLUMP, BRANCH, BACK, STAR, PLUS, CURLY, CURLYN, CURLYM, CURLYX, WHILEM,
    REF, REFF, REFFL, REFFU, REFFA, NREF, NREFF, NREFFL, NREFFU, NREFFA,
    SUSPEND, IFTHEN, BRANCHJ,
    0
};
#endif /* DOINIT */

#ifndef DOINIT
EXTCONST U8 PL_varies_bitmask[];
#else
EXTCONST U8 PL_varies_bitmask[] = {
    0x00, 0x00, 0x00, 0x00, 0x07, 0xF0, 0xE7, 0x7F, 0x26, 0x00, 0x00, 0x00
};
#endif /* DOINIT */

/* The following always have a length of 1. U8 we can do strchr() on it. */
/* (Note that length 1 means "one character" under UTF8, not "one octet".) */
#define REGNODE_SIMPLE(node) (PL_simple_bitmask[(node) >> 3] & (1 << ((node) & 7)))

#ifndef DOINIT
EXTCONST U8 PL_simple[] __attribute__deprecated__;
#else
EXTCONST U8 PL_simple[] __attribute__deprecated__ = {
    REG_ANY, SANY, CANY, ANYOF, ANYOF_WARN_SUPER, ANYOF_SYNTHETIC, POSIXD,
    POSIXL, POSIXU, POSIXA, NPOSIXD, NPOSIXL, NPOSIXU, NPOSIXA,
    0
};
#endif /* DOINIT */

#ifndef DOINIT
EXTCONST U8 PL_simple_bitmask[];
#else
EXTCONST U8 PL_simple_bitmask[] = {
    0x00, 0x00, 0xFC, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif /* DOINIT */

/* ex: set ro: */
