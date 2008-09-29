#!/usr/bin/perl
#
# regen_perly.pl, DAPM 12-Feb-04
#
# Copyright (c) 2004, 2005 Larry Wall
#
# Given an input file perly.y, run bison on it and produce
# the following output files:
#
# perly.h	standard bison header file with minor doctoring of
#		#line directives plus adding a #ifdef PERL_CORE
#
# perly.tab	the parser table C definitions extracted from the bison output
#		plus an extra table generated by this script.
#
# perly.act	the action case statements extracted from the bison output
#
# Note that perly.c is *not* regenerated - this is now a static file which
# is not dependent on perly.y any more.
#
# If a filename of the form foo.y is given on the command line, then
# this is used instead as the basename for all the files mentioned
# above.
#
# Note that temporary files of the form perlytmp.h and perlytmp.c are
# created and then deleted during this process
#
# Note also that this script is intended to be run on a UNIX system;
# it may work elsewhere but no specific attempt has been made to make it
# portable.

sub usage { die "usage: $0 [ -b bison_executable ] [ file.y ]\n" }

use warnings;
use strict;

my $bison = 'bison';

if (@ARGV >= 2 and $ARGV[0] eq '-b') {
    shift;
    $bison = shift;
}

my $y_file = shift || 'perly.y';

usage unless @ARGV==0 && $y_file =~ /\.y$/;

(my $h_file    = $y_file) =~ s/\.y$/.h/;
(my $act_file  = $y_file) =~ s/\.y$/.act/;
(my $tab_file  = $y_file) =~ s/\.y$/.tab/;
(my $tmpc_file = $y_file) =~ s/\.y$/tmp.c/;
(my $tmph_file = $y_file) =~ s/\.y$/tmp.h/;

# the yytranslate[] table generated by bison is ASCII/EBCDIC sensitive

die "$0: must be run on an ASCII system\n" unless ord 'A' == 65;

# check for correct version number. The constraints are:
#  * must be >= 1.24 to avoid licensing issues.
#  * it must generate the yystos[] table. Version 1.28 doesn't generate
#    this; 1.35+ does
#  * Must produce output which is extractable by the regexes below
#  * Must produce the right values.
# These last two contstraints  may well be met by earlier versions, but
# I simply haven't tested them yet. If it works for you, then modify
# the test below to allow that version too. DAPM Feb 04.

my $version = `$bison -V`;
unless ($version =~ /\b(1\.875[a-z]?|2\.[013])\b/) { die <<EOF; }

You have the wrong version of bison in your path; currently 1.875
2.0, 2.1 or 2.3 is required.  Try installing
    http://ftp.gnu.org/gnu/bison/bison-2.1.tar.gz
or similar.  Your bison identifies itself as:

$version
EOF

# creates $tmpc_file and $tmph_file
my_system("$bison -d -o $tmpc_file $y_file");

open CTMPFILE, $tmpc_file or die "Can't open $tmpc_file: $!\n";
my $clines;
{ local $/; $clines = <CTMPFILE>; }
die "failed to read $tmpc_file: length mismatch\n"
    unless length $clines == -s $tmpc_file;
close CTMPFILE;

my ($actlines, $tablines) = extract($clines);

$tablines .= make_type_tab($y_file, $tablines);

chmod 0644, $act_file;
open ACTFILE, ">$act_file" or die "can't open $act_file: $!\n";
print ACTFILE $actlines;
close ACTFILE;
chmod 0444, $act_file;

chmod 0644, $tab_file;
open TABFILE, ">$tab_file" or die "can't open $tab_file: $!\n";
print TABFILE $tablines;
close TABFILE;
chmod 0444, $tab_file;

unlink $tmpc_file;

# Wrap PERL_CORE round the symbol definitions. Also,  the
# C<#line 30 "perly.y"> confuses the Win32 resource compiler and the
# C<#line 188 "perlytmp.h"> gets picked up by make depend, so remove them.

open TMPH_FILE, $tmph_file or die "Can't open $tmph_file: $!\n";
chmod 0644, $h_file;
open H_FILE, ">$h_file" or die "Can't open $h_file: $!\n";
my $endcore_done = 0;
while (<TMPH_FILE>) {
    print H_FILE "#ifdef PERL_CORE\n" if $. == 1;
    if (!$endcore_done and /YYSTYPE_IS_DECLARED/) {
	print H_FILE "#endif /* PERL_CORE */\n";
	$endcore_done = 1;
    }
    next if /^#line \d+ ".*"/;
    print H_FILE $_;
}
close TMPH_FILE;
close H_FILE;
chmod 0444, $h_file;
unlink $tmph_file;

print "rebuilt:  $h_file $tab_file $act_file\n";

exit 0;


sub extract {
    my $clines = shift;
    my $tablines;
    my $actlines;

    $clines =~ m@
	(?:
	    ^/* YYFINAL[^\n]+\n		#optional comment
	)?
	\# \s* define \s* YYFINAL	# first #define
	.*?				# other defines + most tables
	yystos\[\]\s*=			# start of last table
	.*?
	}\s*;				# end of last table
    @xms
	or die "Can't extract tables from $tmpc_file\n";
    $tablines = $&;


    $clines =~ m@
	switch \s* \( \s* \w+ \s* \) \s* { \s*
	(
	    case \s* \d+ \s* : \s*
	    \#line [^\n]+"\Q$y_file\E"
	    .*?
	)
	}
	\s*
	( \s* /\* .*? \*/ \s* )*	# optional C-comments
	\s*
	(
	    \#line[^\n]+\.c"
	|
	    \#line[^\n]+\.simple"
	|
	    YY_SYMBOL_PRINT
	)
    @xms
	or die "Can't extract actions from $tmpc_file\n";
    $actlines = $1;

    # C<#line 188 "perlytmp.c"> gets picked up by make depend, so remove them.
    $actlines =~ s/^#line \d+ "\Q$tmpc_file\E".*$//gm;

    # convert yyvsp[nnn] into ps[nnn].val

    $actlines =~ s/yyvsp\[(.*?)\]/ps[$1].val/g
	or die "Can't convert value stack name\n";

    return $actlines. "\n", $tablines. "\n";
}

# Generate a table, yy_type_tab[], that specifies for each token, what
# type of value it holds.
#
# Read the .y file and extract a list of all the token names and
# non-terminal names; then scan the string $tablines for the table yytname,
# which gives the token index of each token/non-terminal; then use this to
# create yy_type_tab.
#
# ie given (in perly.y),
#
#   %token <opval> A
#   %token <ival>  B
#   %type  <pval>  C
#   %type  <opval> D
#
# and (in $tablines),
#
#   yytname[] = { "A" "B", "C", "D", "E" };
#
# then return
#
#    typedef enum { toketype_ival, toketype_opval, toketype_pval } toketypes;
#
#    static const toketypes yy_type_tab[]
#          = { toketype_opval, toketype_ival, toketype_pval,
#                toketype_opval, toketype_ival }
#
# where "E" has the default type. The default type is determined
# by the __DEFAULT__ comment  next to the appropriate union member in
# perly.y

sub make_type_tab {
    my ($y_file, $tablines) = @_;
    my %tokens;
    my %types;
    my $default_token;
    open my $fh, '<', $y_file or die "Can't open $y_file: $!\n";
    while (<$fh>) {
	if (/(\$\d+)\s*=/) {
	    warn "$y_file:$.: dangerous assignment to $1: $_";
	}

	if (/__DEFAULT__/) {
	    m{(\w+) \s* ; \s* /\* \s* __DEFAULT__}x
		or die "$y_file: can't parse __DEFAULT__ line: $_";
	    die "$y_file: duplicate __DEFAULT__ line: $_"
		    if defined $default_token;
	    $default_token = $1;
	    next;
	}

	next unless /^%(token|type)/;
	s/^%(token|type)\s+<(\w+)>\s+//
	    or die "$y_file: unparseable token/type line: $_";
	$tokens{$_} = $2 for (split ' ', $_);
	$types{$2} = 1;
    }
    die "$y_file: no __DEFAULT__ token defined\n" unless $default_token;
    $types{$default_token} = 1;

    $tablines =~ /^\Qstatic const char *const yytname[] =\E\n
	    {\n
	    (.*?)
	    ^};
	    /xsm
	or die "Can't extract yytname[] from table string\n";
    my $fields = $1;
    $fields =~ s{"([^"]+)"}
		{ "toketype_" .
		    (defined $tokens{$1} ? $tokens{$1} : $default_token)
		}ge;
    $fields =~ s/, \s* 0 \s* $//x
	or die "make_type_tab: couldn't delete trailing ',0'\n";

    return 
	  "\ntypedef enum {\n\t"
	. join(", ", map "toketype_$_", sort keys %types)
	. "\n} toketypes;\n\n"
	. "/* type of each token/terminal */\n"
	. "static const toketypes yy_type_tab[] =\n{\n"
	. $fields
	. "\n};\n";
}


sub my_system {
    system(@_);
    if ($? == -1) {
	die "failed to execute command '@_': $!\n";
    }
    elsif ($? & 127) {
	die sprintf "command '@_' died with signal %d\n",
	    ($? & 127);
    }
    elsif ($? >> 8) {
	die sprintf "command '@_' exited with value %d\n", $? >> 8;
    }
}
