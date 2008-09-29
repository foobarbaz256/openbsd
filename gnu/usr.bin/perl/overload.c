/* -*- buffer-read-only: t -*-
 *
 *    overload.c
 *
 *    Copyright (C) 1997, 1998, 2000, 2001, 2005, 2006, 2007 by Larry Wall
 *    and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 *  !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
 *  This file is built by overload.pl
 */

#define AMG_id2name(id) (PL_AMG_names[id]+1)
#define AMG_id2namelen(id) (PL_AMG_namelens[id]-1)

const U8 PL_AMG_namelens[NofAMmeth] = {
    2,
    4,
    4,
    4,
    4,
    4,
    3,
    3,
    5,
    3,
    3,
    2,
    2,
    4,
    4,
    3,
    4,
    2,
    3,
    2,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    9,
    2,
    3,
    2,
    3,
    2,
    3,
    2,
    3,
    2,
    3,
    3,
    4,
    3,
    4,
    3,
    4,
    2,
    3,
    2,
    3,
    2,
    3,
    4,
    4,
    2,
    6,
    4,
    4,
    4,
    4,
    5,
    2,
    3,
    2,
    3,
    3,
    7
};

char * const PL_AMG_names[NofAMmeth] = {
  /* Names kept in the symbol table.  fallback => "()", the rest has
     "(" prepended.  The only other place in perl which knows about
     this convention is AMG_id2name (used for debugging output and
     'nomethod' only), the only other place which has it hardwired is
     overload.pm.  */
    "()",
    "(${}",
    "(@{}",
    "(%{}",
    "(*{}",
    "(&{}",
    "(++",
    "(--",
    "(bool",
    "(0+",
    "(\"\"",
    "(!",
    "(=",
    "(abs",
    "(neg",
    "(<>",
    "(int",
    "(<",
    "(<=",
    "(>",
    "(>=",
    "(==",
    "(!=",
    "(lt",
    "(le",
    "(gt",
    "(ge",
    "(eq",
    "(ne",
    "(nomethod",
    "(+",
    "(+=",
    "(-",
    "(-=",
    "(*",
    "(*=",
    "(/",
    "(/=",
    "(%",
    "(%=",
    "(**",
    "(**=",
    "(<<",
    "(<<=",
    "(>>",
    "(>>=",
    "(&",
    "(&=",
    "(|",
    "(|=",
    "(^",
    "(^=",
    "(<=>",
    "(cmp",
    "(~",
    "(atan2",
    "(cos",
    "(sin",
    "(exp",
    "(log",
    "(sqrt",
    "(x",
    "(x=",
    "(.",
    "(.=",
    "(~~",
    "DESTROY"
};
