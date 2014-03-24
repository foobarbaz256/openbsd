package autodie::hints;

use strict;
use warnings;

use constant PERL58 => ( $] < 5.009 );

our $VERSION = '2.13';

=head1 NAME

autodie::hints - Provide hints about user subroutines to autodie

=head1 SYNOPSIS

    package Your::Module;

    our %DOES = ( 'autodie::hints::provider' => 1 );

    sub AUTODIE_HINTS {
        return {
            foo => { scalar => HINTS, list => SOME_HINTS },
            bar => { scalar => HINTS, list => MORE_HINTS },
        }
    }

    # Later, in your main program...

    use Your::Module qw(foo bar);
    use autodie      qw(:default foo bar);

    foo();         # succeeds or dies based on scalar hints

    # Alternatively, hints can be set on subroutines we've
    # imported.

    use autodie::hints;
    use Some::Module qw(think_positive);

    BEGIN {
        autodie::hints->set_hints_for(
            \&think_positive,
            {
                fail => sub { $_[0] <= 0 }
            }
        )
    }
    use autodie qw(think_positive);

    think_positive(...);    # Returns positive or dies.


=head1 DESCRIPTION

=head2 Introduction

The L<autodie> pragma is very smart when it comes to working with
Perl's built-in functions.  The behaviour for these functions are
fixed, and C<autodie> knows exactly how they try to signal failure.

But what about user-defined subroutines from modules?  If you use
C<autodie> on a user-defined subroutine then it assumes the following
behaviour to demonstrate failure:

=over

=item *

A false value, in scalar context

=item * 

An empty list, in list context

=item *

A list containing a single undef, in list context

=back

All other return values (including the list of the single zero, and the
list containing a single empty string) are considered successful.  However,
real-world code isn't always that easy.  Perhaps the code you're working
with returns a string containing the word "FAIL" upon failure, or a
two element list containing C<(undef, "human error message")>.  To make
autodie work with these sorts of subroutines, we have
the I<hinting interface>.

The hinting interface allows I<hints> to be provided to C<autodie>
on how it should detect failure from user-defined subroutines.  While
these I<can> be provided by the end-user of C<autodie>, they are ideally
written into the module itself, or into a helper module or sub-class
of C<autodie> itself.

=head2 What are hints?

A I<hint> is a subroutine or value that is checked against the
return value of an autodying subroutine.  If the match returns true,
C<autodie> considers the subroutine to have failed.

If the hint provided is a subroutine, then C<autodie> will pass
the complete return value to that subroutine.  If the hint is
any other value, then C<autodie> will smart-match against the
value provided.  In Perl 5.8.x there is no smart-match operator, and as such
only subroutine hints are supported in these versions.

Hints can be provided for both scalar and list contexts.  Note
that an autodying subroutine will never see a void context, as
C<autodie> always needs to capture the return value for examination.
Autodying subroutines called in void context act as if they're called
in a scalar context, but their return value is discarded after it
has been checked.

=head2 Example hints

Hints may consist of scalars, array references, regular expressions and
subroutine references.  You can specify different hints for how
failure should be identified in scalar and list contexts.

These examples apply for use in the C<AUTODIE_HINTS> subroutine and when
calling C<autodie::hints->set_hints_for()>.

The most common context-specific hints are:

        # Scalar failures always return undef:
            {  scalar => undef  }

        # Scalar failures return any false value [default expectation]:
            {  scalar => sub { ! $_[0] }  }

        # Scalar failures always return zero explicitly:
            {  scalar => '0'  }

        # List failures always return an empty list:
            {  list => []  }

        # List failures return () or (undef) [default expectation]:
            {  list => sub { ! @_ || @_ == 1 && !defined $_[0] }  }

        # List failures return () or a single false value:
            {  list => sub { ! @_ || @_ == 1 && !$_[0] }  }

        # List failures return (undef, "some string")
            {  list => sub { @_ == 2 && !defined $_[0] }  }

        # Unsuccessful foo() returns 'FAIL' or '_FAIL' in scalar context,
        #                    returns (-1) in list context...
        autodie::hints->set_hints_for(
            \&foo,
            {
                scalar => qr/^ _? FAIL $/xms,
                list   => [-1],
            }
        );

        # Unsuccessful foo() returns 0 in all contexts...
        autodie::hints->set_hints_for(
            \&foo,
            {
                scalar => 0,
                list   => [0],
            }
        );

This "in all contexts" construction is very common, and can be
abbreviated, using the 'fail' key.  This sets both the C<scalar>
and C<list> hints to the same value:

        # Unsuccessful foo() returns 0 in all contexts...
        autodie::hints->set_hints_for(
            \&foo,
            {
                fail => sub { @_ == 1 and defined $_[0] and $_[0] == 0 }
            }
	);

        # Unsuccessful think_positive() returns negative number on failure...
        autodie::hints->set_hints_for(
            \&think_positive,
            {
                fail => sub { $_[0] < 0 }
            }
	);

        # Unsuccessful my_system() returns non-zero on failure...
        autodie::hints->set_hints_for(
            \&my_system,
            {
                fail => sub { $_[0] != 0 }
            }
	);

=head1 Manually setting hints from within your program

If you are using a module which returns something special on failure, then
you can manually create hints for each of the desired subroutines.  Once
the hints are specified, they are available for all files and modules loaded
thereafter, thus you can move this work into a module and it will still
work.

	use Some::Module qw(foo bar);
	use autodie::hints;

	autodie::hints->set_hints_for(
		\&foo,
		{
			scalar => SCALAR_HINT,
			list   => LIST_HINT,
		}
	);
	autodie::hints->set_hints_for(
		\&bar,
                { fail => SOME_HINT, }
	);

It is possible to pass either a subroutine reference (recommended) or a fully
qualified subroutine name as the first argument.  This means you can set hints
on modules that I<might> get loaded:

	use autodie::hints;
	autodie::hints->set_hints_for(
		'Some::Module:bar', { fail => SCALAR_HINT, }
	);

This technique is most useful when you have a project that uses a
lot of third-party modules.  You can define all your possible hints
in one-place.  This can even be in a sub-class of autodie.  For
example:

        package my::autodie;

        use parent qw(autodie);
        use autodie::hints;

        autodie::hints->set_hints_for(...);

        1;

You can now C<use my::autodie>, which will work just like the standard
C<autodie>, but is now aware of any hints that you've set.

=head1 Adding hints to your module

C<autodie> provides a passive interface to allow you to declare hints for
your module.  These hints will be found and used by C<autodie> if it
is loaded, but otherwise have no effect (or dependencies) without autodie.
To set these, your module needs to declare that it I<does> the
C<autodie::hints::provider> role.  This can be done by writing your
own C<DOES> method, using a system such as C<Class::DOES> to handle
the heavy-lifting for you, or declaring a C<%DOES> package variable
with a C<autodie::hints::provider> key and a corresponding true value.

Note that checking for a C<%DOES> hash is an C<autodie>-only
short-cut.  Other modules do not use this mechanism for checking
roles, although you can use the C<Class::DOES> module from the
CPAN to allow it.

In addition, you must define a C<AUTODIE_HINTS> subroutine that returns
a hash-reference containing the hints for your subroutines:

        package Your::Module;

        # We can use the Class::DOES from the CPAN to declare adherence
        # to a role.

        use Class::DOES 'autodie::hints::provider' => 1;

        # Alternatively, we can declare the role in %DOES.  Note that
        # this is an autodie specific optimisation, although Class::DOES
        # can be used to promote this to a true role declaration.

        our %DOES = ( 'autodie::hints::provider' => 1 );

        # Finally, we must define the hints themselves.

	sub AUTODIE_HINTS {
	    return {
	        foo => { scalar => HINTS, list => SOME_HINTS },
	        bar => { scalar => HINTS, list => MORE_HINTS },
	        baz => { fail => HINTS },
	    }
	}

This allows your code to set hints without relying on C<autodie> and
C<autodie::hints> being loaded, or even installed.  In this way your
code can do the right thing when C<autodie> is installed, but does not
need to depend upon it to function.

=head1 Insisting on hints

When a user-defined subroutine is wrapped by C<autodie>, it will
use hints if they are available, and otherwise reverts to the
I<default behaviour> described in the introduction of this document.
This can be problematic if we expect a hint to exist, but (for
whatever reason) it has not been loaded.

We can ask autodie to I<insist> that a hint be used by prefixing
an exclamation mark to the start of the subroutine name.  A lone
exclamation mark indicates that I<all> subroutines after it must
have hints declared.

	# foo() and bar() must have their hints defined
	use autodie qw( !foo !bar baz );

	# Everything must have hints (recommended).
	use autodie qw( ! foo bar baz );

	# bar() and baz() must have their hints defined
	use autodie qw( foo ! bar baz );

        # Enable autodie for all of Perl's supported built-ins,
        # as well as for foo(), bar() and baz().  Everything must
        # have hints.
        use autodie qw( ! :all foo bar baz );

If hints are not available for the specified subroutines, this will cause a
compile-time error.  Insisting on hints for Perl's built-in functions
(eg, C<open> and C<close>) is always successful.

Insisting on hints is I<strongly> recommended.

=cut

# TODO: implement regular expression hints

use constant UNDEF_ONLY       => sub { not defined $_[0] };
use constant EMPTY_OR_UNDEF   => sub {
    ! @_ or
    @_==1 && !defined $_[0]
};

use constant EMPTY_ONLY     => sub { @_ == 0 };
use constant EMPTY_OR_FALSE => sub {
    ! @_ or
    @_==1 && !$_[0]
};

use constant SINGLE_TRUE => sub { @_ == 1 and not $_[0] };

use constant DEFAULT_HINTS => {
    scalar => UNDEF_ONLY,
    list   => EMPTY_OR_UNDEF,
};


use constant HINTS_PROVIDER => 'autodie::hints::provider';

use base qw(Exporter);

our $DEBUG = 0;

# Only ( undef ) is a strange but possible situation for very
# badly written code.  It's not supported yet.

my %Hints = (
    'File::Copy::copy' => { scalar => SINGLE_TRUE, list => SINGLE_TRUE },
    'File::Copy::move' => { scalar => SINGLE_TRUE, list => SINGLE_TRUE },
    'File::Copy::cp'   => { scalar => SINGLE_TRUE, list => SINGLE_TRUE },
    'File::Copy::mv'   => { scalar => SINGLE_TRUE, list => SINGLE_TRUE },
);

# Start by using Sub::Identify if it exists on this system.

eval { require Sub::Identify; Sub::Identify->import('get_code_info'); };

# If it doesn't exist, we'll define our own.  This code is directly
# taken from Rafael Garcia's Sub::Identify 0.04, used under the same
# license as Perl itself.

if ($@) {
    require B;

    no warnings 'once';

    *get_code_info = sub ($) {

        my ($coderef) = @_;
        ref $coderef or return;
        my $cv = B::svref_2object($coderef);
        $cv->isa('B::CV') or return;
        # bail out if GV is undefined
        $cv->GV->isa('B::SPECIAL') and return;

        return ($cv->GV->STASH->NAME, $cv->GV->NAME);
    };

}

sub sub_fullname {
    return join( '::', get_code_info( $_[1] ) );
}

my %Hints_loaded = ();

sub load_hints {
    my ($class, $sub) = @_;

    my ($package) = ( $sub =~ /(.*)::/ );

    if (not defined $package) {
        require Carp;
        Carp::croak(
            "Internal error in autodie::hints::load_hints - no package found.
        ");
    }

    # Do nothing if we've already tried to load hints for
    # this package.
    return if $Hints_loaded{$package}++;

    my $hints_available = 0;

    {
        no strict 'refs';   ## no critic

        if ($package->can('DOES') and $package->DOES(HINTS_PROVIDER) ) {
            $hints_available = 1;
        }
        elsif ( PERL58 and $package->isa(HINTS_PROVIDER) ) {
            $hints_available = 1;
        }
        elsif ( ${"${package}::DOES"}{HINTS_PROVIDER.""} ) {
            $hints_available = 1;
        }
    }

    return if not $hints_available;

    my %package_hints = %{ $package->AUTODIE_HINTS };

    foreach my $sub (keys %package_hints) {

        my $hint = $package_hints{$sub};

        # Ensure we have a package name.
        $sub = "${package}::$sub" if $sub !~ /::/;

        # TODO - Currently we don't check for conflicts, should we?
        $Hints{$sub} = $hint;

        $class->normalise_hints(\%Hints, $sub);
    }

    return;

}

sub normalise_hints {
    my ($class, $hints, $sub) = @_;

    if ( exists $hints->{$sub}->{fail} ) {

        if ( exists $hints->{$sub}->{scalar} or
             exists $hints->{$sub}->{list}
        ) {
            # TODO: Turn into a proper diagnostic.
            require Carp;
            local $Carp::CarpLevel = 1;
            Carp::croak("fail hints cannot be provided with either scalar or list hints for $sub");
        }

        # Set our scalar and list hints.

        $hints->{$sub}->{scalar} = 
        $hints->{$sub}->{list} = delete $hints->{$sub}->{fail};

        return;

    }

    # Check to make sure all our hints exist.

    foreach my $hint (qw(scalar list)) {
        if ( not exists $hints->{$sub}->{$hint} ) {
            # TODO: Turn into a proper diagnostic.
            require Carp;
            local $Carp::CarpLevel = 1;
            Carp::croak("$hint hint missing for $sub");
        }
    }

    return;
}

sub get_hints_for {
    my ($class, $sub) = @_;

    my $subname = $class->sub_fullname( $sub );

    # If we have hints loaded for a sub, then return them.

    if ( exists $Hints{ $subname } ) {
        return $Hints{ $subname };
    }

    # If not, we try to load them...

    $class->load_hints( $subname );

    # ...and try again!

    if ( exists $Hints{ $subname } ) {
        return $Hints{ $subname };
    }

    # It's the caller's responsibility to use defaults if desired.
    # This allows on autodie to insist on hints if needed.

    return;

}

sub set_hints_for {
    my ($class, $sub, $hints) = @_;

    if (ref $sub) {
        $sub = $class->sub_fullname( $sub );

        require Carp;

        $sub or Carp::croak("Attempts to set_hints_for unidentifiable subroutine");
    }

    if ($DEBUG) {
        warn "autodie::hints: Setting $sub to hints: $hints\n";
    }

    $Hints{ $sub } = $hints;

    $class->normalise_hints(\%Hints, $sub);

    return;
}

1;

__END__


=head1 Diagnostics

=over 4

=item Attempts to set_hints_for unidentifiable subroutine

You've called C<< autodie::hints->set_hints_for() >> using a subroutine
reference, but that reference could not be resolved back to a
subroutine name.  It may be an anonymous subroutine (which can't
be made autodying), or may lack a name for other reasons.

If you receive this error with a subroutine that has a real name,
then you may have found a bug in autodie.  See L<autodie/BUGS>
for how to report this.

=item fail hints cannot be provided with either scalar or list hints for %s

When defining hints, you can either supply both C<list> and
C<scalar> keywords, I<or> you can provide a single C<fail> keyword.
You can't mix and match them.

=item %s hint missing for %s

You've provided either a C<scalar> hint without supplying
a C<list> hint, or vice-versa.  You I<must> supply both C<scalar>
and C<list> hints, I<or> a single C<fail> hint.

=back

=head1 ACKNOWLEDGEMENTS

=over 

=item *

Dr Damian Conway for suggesting the hinting interface and providing the
example usage.

=item *

Jacinta Richardson for translating much of my ideas into this
documentation.

=back

=head1 AUTHOR

Copyright 2009, Paul Fenwick E<lt>pjf@perltraining.com.auE<gt>

=head1 LICENSE

This module is free software.  You may distribute it under the
same terms as Perl itself.

=head1 SEE ALSO

L<autodie>, L<Class::DOES>

=cut
