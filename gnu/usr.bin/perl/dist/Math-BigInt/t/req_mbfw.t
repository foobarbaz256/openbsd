#!/usr/bin/perl -w

# check that requiring BigFloat and then calling import() works

use strict;
use Test::More tests => 3;

BEGIN { unshift @INC, 't'; }

# normal require that calls import automatically (we thus have MBI afterwards)
require Math::BigFloat;
my $x = Math::BigFloat->new(1); ++$x;
is ($x,2, '$x is 2');

like (Math::BigFloat->config()->{with}, qr/^Math::BigInt::(Fast)?Calc\z/, 'with ignored' );

# now override
Math::BigFloat->import ( with => 'Math::BigInt::Subclass' );

# the "with" argument is ignored
like (Math::BigFloat->config()->{with}, qr/^Math::BigInt::(Fast)?Calc\z/, 'with ignored' );

# all tests done
