#!perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
    if (!$Config::Config{useperlio}) {
        print "1..0 # Skip -- need perlio to walk the optree\n";
        exit 0;
    }
}

use OptreeCheck;	# ALSO DOES @ARGV HANDLING !!!!!!
use Config;

plan tests => 67;

#################################

use constant {		# see also t/op/gv.t line 358
    myaref	=> [ 1,2,3 ],
    myfl	=> 1.414213,
    myglob	=> \*STDIN,
    myhref	=> { a	=> 1 },
    myint	=> 42,
    myrex	=> qr/foo/,
    mystr	=> 'hithere',
    mysub	=> \&ok,
    myundef	=> undef,
    myunsub	=> \&nosuch,
};

sub myyes() { 1==1 }
sub myno () { return 1!=1 }
sub pi () { 3.14159 };

my $RV_class = $] >= 5.011 ? 'IV' : 'RV';

my $want = {	# expected types, how value renders in-line, todos (maybe)
    mystr	=> [ 'PV', '"'.mystr.'"' ],
    myhref	=> [ $RV_class, '\\\\HASH'],
    pi		=> [ 'NV', pi ],
    myglob	=> [ $RV_class, '\\\\' ],
    mysub	=> [ $RV_class, '\\\\' ],
    myunsub	=> [ $RV_class, '\\\\' ],
    # these are not inlined, at least not per BC::Concise
    #myyes	=> [ $RV_class, ],
    #myno	=> [ $RV_class, ],
    myaref	=> [ $RV_class, '\\\\' ],
    myfl	=> [ 'NV', myfl ],
    myint	=> [ 'IV', myint ],
    $] >= 5.011 ? (
    myrex	=> [ $RV_class, '\\\\"\\(?^:Foo\\)"' ],
    ) : (
    myrex	=> [ $RV_class, '\\\\' ],
    ),
    myundef	=> [ 'NULL', ],
};

use constant WEEKDAYS
    => qw ( Sunday Monday Tuesday Wednesday Thursday Friday Saturday );


$::{napier} = \2.71828;	# counter-example (doesn't get optimized).
eval "sub napier ();";


# should be able to undefine constant::import here ???
INIT { 
    # eval 'sub constant::import () {}';
    # undef *constant::import::{CODE};
};

#################################
pass("RENDER CONSTANT SUBS RETURNING SCALARS");

for $func (sort keys %$want) {
    # no strict 'refs';	# why not needed ?
    checkOptree ( name      => "$func() as a coderef",
		  code      => \&{$func},
		  noanchors => 1,
		  expect    => <<EOT_EOT, expect_nt => <<EONT_EONT);
 is a constant sub, optimized to a $want->{$func}[0]
EOT_EOT
 is a constant sub, optimized to a $want->{$func}[0]
EONT_EONT

}

pass("RENDER CALLS TO THOSE CONSTANT SUBS");

for $func (sort keys %$want) {
    # print "# doing $func\n";
    checkOptree ( name    => "call $func",
		  code    => "$func",
		  ($want->{$func}[2]) ? ( todo => $want->{$func}[2]) : (),
		  bc_opts => '-nobanner',
		  expect  => <<EOT_EOT, expect_nt => <<EONT_EONT);
3  <1> leavesub[2 refs] K/REFC,1 ->(end)
-     <\@> lineseq KP ->3
1        <;> dbstate(main 833 (eval 44):1) v ->2
2        <\$> const[$want->{$func}[0] $want->{$func}[1]] s* ->3      < 5.017002
2        <\$> const[$want->{$func}[0] $want->{$func}[1]] s*/FOLD ->3 >=5.017002
EOT_EOT
3  <1> leavesub[2 refs] K/REFC,1 ->(end)
-     <\@> lineseq KP ->3
1        <;> dbstate(main 833 (eval 44):1) v ->2
2        <\$> const($want->{$func}[0] $want->{$func}[1]) s* ->3      < 5.017002
2        <\$> const($want->{$func}[0] $want->{$func}[1]) s*/FOLD ->3 >=5.017002
EONT_EONT

}

##############
pass("MORE TESTS");

checkOptree ( name	=> 'myyes() as coderef',
	      code	=> sub () { 1==1 },
	      noanchors => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
 is a constant sub, optimized to a SPECIAL
EOT_EOT
 is a constant sub, optimized to a SPECIAL
EONT_EONT


checkOptree ( name	=> 'myyes() as coderef',
	      prog	=> 'sub a() { 1==1 }; print a',
	      noanchors => 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const[SPECIAL sv_yes] s* ->5         < 5.017002
# 4        <$> const[SPECIAL sv_yes] s*/FOLD ->5    >=5.017002
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const(SPECIAL sv_yes) s* ->5         < 5.017002
# 4        <$> const(SPECIAL sv_yes) s*/FOLD ->5    >=5.017002
EONT_EONT


# Need to do this as a prog, not code, as only the first constant to use
# PL_sv_no actually gets to use the real thing - every one following is
# copied.
checkOptree ( name	=> 'myno() as coderef',
	      prog	=> 'sub a() { 1!=1 }; print a',
	      noanchors => 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const[SPECIAL sv_no] s* ->5         < 5.017002
# 4        <$> const[SPECIAL sv_no] s*/FOLD ->5    >=5.017002
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const(SPECIAL sv_no) s* ->5         < 5.017002
# 4        <$> const(SPECIAL sv_no) s*/FOLD ->5    >=5.017002
EONT_EONT


my ($expect, $expect_nt) =
    $] >= 5.019003
	? (" is a constant sub, optimized to a AV\n") x 2
	: (<<'EOT_EOT', <<'EONT_EONT');
# 3  <1> leavesub[2 refs] K/REFC,1 ->(end)
# -     <@> lineseq K ->3
# 1        <;> nextstate(constant 61 constant.pm:118) v:*,&,x*,x&,x$ ->2
# 2        <0> padav[@list:FAKE:m:96] ->3
EOT_EOT
# 3  <1> leavesub[2 refs] K/REFC,1 ->(end)
# -     <@> lineseq K ->3
# 1        <;> nextstate(constant 61 constant.pm:118) v:*,&,x*,x&,x$ ->2
# 2        <0> padav[@list:FAKE:m:71] ->3
EONT_EONT


checkOptree ( name	=> 'constant sub returning list',
	      code	=> \&WEEKDAYS,
	      noanchors => 1,
	      expect => $expect, expect_nt => $expect_nt);


sub printem {
    printf "myint %d mystr %s myfl %f pi %f\n"
	, myint, mystr, myfl, pi;
}

my ($expect, $expect_nt) = (<<'EOT_EOT', <<'EONT_EONT');
# 9  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->9
# 1        <;> nextstate(main 635 optree_constants.t:163) v:>,<,% ->2
# 8        <@> prtf sK ->9
# 2           <0> pushmark sM ->3
# 3           <$> const[PV "myint %d mystr %s myfl %f pi %f\n"] sM/FOLD ->4
# 4           <$> const[IV 42] sM* ->5          < 5.017002
# 5           <$> const[PV "hithere"] sM* ->6   < 5.017002
# 6           <$> const[NV 1.414213] sM* ->7    < 5.017002
# 7           <$> const[NV 3.14159] sM* ->8     < 5.017002
# 4           <$> const[IV 42] sM*/FOLD ->5          >=5.017002 
# 5           <$> const[PV "hithere"] sM*/FOLD ->6   >=5.017002
# 6           <$> const[NV 1.414213] sM*/FOLD ->7    >=5.017002
# 7           <$> const[NV 3.14159] sM*/FOLD ->8     >=5.017002
EOT_EOT
# 9  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->9
# 1        <;> nextstate(main 635 optree_constants.t:163) v:>,<,% ->2
# 8        <@> prtf sK ->9
# 2           <0> pushmark sM ->3
# 3           <$> const(PV "myint %d mystr %s myfl %f pi %f\n") sM/FOLD ->4
# 4           <$> const(IV 42) sM* ->5          < 5.017002
# 5           <$> const(PV "hithere") sM* ->6   < 5.017002
# 6           <$> const(NV 1.414213) sM* ->7    < 5.017002
# 7           <$> const(NV 3.14159) sM* ->8     < 5.017002
# 4           <$> const(IV 42) sM*/FOLD ->5          >=5.017002 
# 5           <$> const(PV "hithere") sM*/FOLD ->6   >=5.017002
# 6           <$> const(NV 1.414213) sM*/FOLD ->7    >=5.017002
# 7           <$> const(NV 3.14159) sM*/FOLD ->8     >=5.017002
EONT_EONT

if($] < 5.015) {
    s/M(?=\*? ->)//g for $expect, $expect_nt;
}
if($] < 5.017002 || $] >= 5.019004) {
    s|\\n"[])] sM\K/FOLD|| for $expect, $expect_nt;
}

checkOptree ( name	=> 'call many in a print statement',
	      code	=> \&printem,
	      strip_open_hints => 1,
	      expect => $expect, expect_nt => $expect_nt);

# test constant expression folding

checkOptree ( name	=> 'arithmetic constant folding in print',
	      code	=> 'print 1+2+3',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 937 (eval 53):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[IV 6] s ->4      < 5.017002
# 3           <$> const[IV 6] s/FOLD ->4 >=5.017002
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 937 (eval 53):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(IV 6) s ->4      < 5.017002
# 3           <$> const(IV 6) s/FOLD ->4 >=5.017002
EONT_EONT

checkOptree ( name	=> 'string constant folding in print',
	      code	=> 'print "foo"."bar"',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar"] s ->4      < 5.017002
# 3           <$> const[PV "foobar"] s/FOLD ->4 >=5.017002
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar") s ->4      < 5.017002
# 3           <$> const(PV "foobar") s/FOLD ->4 >=5.017002
EONT_EONT

checkOptree ( name	=> 'boolean or folding',
	      code	=> 'print "foobar" if 1 or 0',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5      < 5.019004
# 4        <@> print sK/FOLD ->5 >=5.019004
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar"] s ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5      < 5.019004
# 4        <@> print sK/FOLD ->5 >=5.019004
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar") s ->4
EONT_EONT

checkOptree ( name	=> 'lc*,uc*,gt,lt,ge,le,cmp',
	      code	=> sub {
		  $s = uc('foo.').ucfirst('bar.').lc('LOW.').lcfirst('LOW');
		  print "a-lt-b" if "a" lt "b";
		  print "b-gt-a" if "b" gt "a";
		  print "a-le-b" if "a" le "b";
		  print "b-ge-a" if "b" ge "a";
		  print "b-cmp-a" if "b" cmp "a";
		  print "a-gt-b" if "a" gt "b";	# should be suppressed
	      },
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# 1        <;> nextstate(main 916 optree_constants.t:307) v:>,<,%,{ ->2
# 4        <2> sassign vKS/2 ->5
# 2           <$> const[PV "FOO.Bar.low.lOW"] s ->3      < 5.017002
# 2           <$> const[PV "FOO.Bar.low.lOW"] s/FOLD ->3 >=5.017002
# -           <1> ex-rv2sv sKRM*/1 ->4
# 3              <#> gvsv[*s] s ->4
# 5        <;> nextstate(main 916 optree_constants.t:308) v:>,<,%,{ ->6
# 8        <@> print vK ->9      < 5.019004
# 8        <@> print vK/FOLD ->9 >=5.019004
# 6           <0> pushmark s ->7
# 7           <$> const[PV "a-lt-b"] s ->8
# 9        <;> nextstate(main 916 optree_constants.t:309) v:>,<,%,{ ->a
# c        <@> print vK ->d      < 5.019004
# c        <@> print vK/FOLD ->d >=5.019004
# a           <0> pushmark s ->b
# b           <$> const[PV "b-gt-a"] s ->c
# d        <;> nextstate(main 916 optree_constants.t:310) v:>,<,%,{ ->e
# g        <@> print vK ->h      < 5.019004
# g        <@> print vK/FOLD ->h >=5.019004
# e           <0> pushmark s ->f
# f           <$> const[PV "a-le-b"] s ->g
# h        <;> nextstate(main 916 optree_constants.t:311) v:>,<,%,{ ->i
# k        <@> print vK ->l      < 5.019004
# k        <@> print vK/FOLD ->l >=5.019004
# i           <0> pushmark s ->j
# j           <$> const[PV "b-ge-a"] s ->k
# l        <;> nextstate(main 916 optree_constants.t:312) v:>,<,%,{ ->m
# o        <@> print vK ->p      < 5.019004
# o        <@> print vK/FOLD ->p >=5.019004
# m           <0> pushmark s ->n
# n           <$> const[PV "b-cmp-a"] s ->o
# p        <;> nextstate(main 916 optree_constants.t:313) v:>,<,%,{ ->q
# q        <$> const[PVNV 0] s/SHORT ->r      < 5.017002
# q        <$> const[PVNV 0] s/FOLD,SHORT ->r >=5.017002 < 5.019003
# q        <$> const[SPECIAL sv_no] s/SHORT,FOLD ->r >=5.019003
EOT_EOT
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# 1        <;> nextstate(main 916 optree_constants.t:307) v:>,<,%,{ ->2
# 4        <2> sassign vKS/2 ->5
# 2           <$> const(PV "FOO.Bar.low.lOW") s ->3      < 5.017002
# 2           <$> const(PV "FOO.Bar.low.lOW") s/FOLD ->3 >=5.017002
# -           <1> ex-rv2sv sKRM*/1 ->4
# 3              <$> gvsv(*s) s ->4
# 5        <;> nextstate(main 916 optree_constants.t:308) v:>,<,%,{ ->6
# 8        <@> print vK ->9      < 5.019004
# 8        <@> print vK/FOLD ->9 >=5.019004
# 6           <0> pushmark s ->7
# 7           <$> const(PV "a-lt-b") s ->8
# 9        <;> nextstate(main 916 optree_constants.t:309) v:>,<,%,{ ->a
# c        <@> print vK ->d      < 5.019004
# c        <@> print vK/FOLD ->d >=5.019004
# a           <0> pushmark s ->b
# b           <$> const(PV "b-gt-a") s ->c
# d        <;> nextstate(main 916 optree_constants.t:310) v:>,<,%,{ ->e
# g        <@> print vK ->h      < 5.019004
# g        <@> print vK/FOLD ->h >=5.019004
# e           <0> pushmark s ->f
# f           <$> const(PV "a-le-b") s ->g
# h        <;> nextstate(main 916 optree_constants.t:311) v:>,<,%,{ ->i
# k        <@> print vK ->l      < 5.019004
# k        <@> print vK/FOLD ->l >=5.019004
# i           <0> pushmark s ->j
# j           <$> const(PV "b-ge-a") s ->k
# l        <;> nextstate(main 916 optree_constants.t:312) v:>,<,%,{ ->m
# o        <@> print vK ->p      < 5.019004
# o        <@> print vK/FOLD ->p >=5.019004
# m           <0> pushmark s ->n
# n           <$> const(PV "b-cmp-a") s ->o
# p        <;> nextstate(main 916 optree_constants.t:313) v:>,<,%,{ ->q
# q        <$> const(SPECIAL sv_no) s/SHORT ->r      < 5.017002
# q        <$> const(SPECIAL sv_no) s/SHORT,FOLD ->r >=5.017002
EONT_EONT

checkOptree ( name	=> 'mixed constant folding, with explicit braces',
	      code	=> 'print "foo"."bar".(2+3)',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 977 (eval 28):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar5"] s ->4      < 5.017002
# 3           <$> const[PV "foobar5"] s/FOLD ->4 >=5.017002
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 977 (eval 28):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar5") s ->4      < 5.017002
# 3           <$> const(PV "foobar5") s/FOLD ->4 >=5.017002
EONT_EONT

__END__

=head NB

Optimized constant subs are stored as bare scalars in the stash
(package hash), which formerly held only GVs (typeglobs).

But you cant create them manually - you cant assign a scalar to a
stash element, and expect it to work like a constant-sub, even if you
provide a prototype.

This is a feature; alternative is too much action-at-a-distance.  The
following test demonstrates - napier is not seen as a function at all,
much less an optimized one.

=cut

checkOptree ( name	=> 'not evertnapier',
	      code	=> \&napier,
	      noanchors => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
 has no START
EOT_EOT
 has no START
EONT_EONT


