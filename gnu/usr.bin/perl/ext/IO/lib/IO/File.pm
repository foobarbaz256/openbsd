#

package IO::File;

=head1 NAME

IO::File - supply object methods for filehandles

=head1 SYNOPSIS

    use IO::File;

    $fh = new IO::File;
    if ($fh->open("< file")) {
        print <$fh>;
        $fh->close;
    }

    $fh = new IO::File "> file";
    if (defined $fh) {
        print $fh "bar\n";
        $fh->close;
    }

    $fh = new IO::File "file", "r";
    if (defined $fh) {
        print <$fh>;
        undef $fh;       # automatically closes the file
    }

    $fh = new IO::File "file", O_WRONLY|O_APPEND;
    if (defined $fh) {
        print $fh "corge\n";

        $pos = $fh->getpos;
        $fh->setpos($pos);

        undef $fh;       # automatically closes the file
    }

    autoflush STDOUT 1;

=head1 DESCRIPTION

C<IO::File> inherits from C<IO::Handle> and C<IO::Seekable>. It extends
these classes with methods that are specific to file handles.

=head1 CONSTRUCTOR

=over 4

=item new ( FILENAME [,MODE [,PERMS]] )

Creates an C<IO::File>.  If it receives any parameters, they are passed to
the method C<open>; if the open fails, the object is destroyed.  Otherwise,
it is returned to the caller.

=item new_tmpfile

Creates an C<IO::File> opened for read/write on a newly created temporary
file.  On systems where this is possible, the temporary file is anonymous
(i.e. it is unlinked after creation, but held open).  If the temporary
file cannot be created or opened, the C<IO::File> object is destroyed.
Otherwise, it is returned to the caller.

=back

=head1 METHODS

=over 4

=item open( FILENAME [,MODE [,PERMS]] )

=item open( FILENAME, IOLAYERS )

C<open> accepts one, two or three parameters.  With one parameter,
it is just a front end for the built-in C<open> function.  With two or three
parameters, the first parameter is a filename that may include
whitespace or other special characters, and the second parameter is
the open mode, optionally followed by a file permission value.

If C<IO::File::open> receives a Perl mode string ("E<gt>", "+E<lt>", etc.)
or an ANSI C fopen() mode string ("w", "r+", etc.), it uses the basic
Perl C<open> operator (but protects any special characters).

If C<IO::File::open> is given a numeric mode, it passes that mode
and the optional permissions value to the Perl C<sysopen> operator.
The permissions default to 0666.

If C<IO::File::open> is given a mode that includes the C<:> character,
it passes all the three arguments to the three-argument C<open> operator.

For convenience, C<IO::File> exports the O_XXX constants from the
Fcntl module, if this module is available.

=back

=head1 SEE ALSO

L<perlfunc>, 
L<perlop/"I/O Operators">,
L<IO::Handle>
L<IO::Seekable>

=head1 HISTORY

Derived from FileHandle.pm by Graham Barr E<lt>F<gbarr@pobox.com>E<gt>.

=cut

use 5.006_001;
use strict;
our($VERSION, @EXPORT, @EXPORT_OK, @ISA);
use Carp;
use Symbol;
use SelectSaver;
use IO::Seekable;
use File::Spec;

require Exporter;

@ISA = qw(IO::Handle IO::Seekable Exporter);

$VERSION = "1.10";

@EXPORT = @IO::Seekable::EXPORT;

eval {
    # Make all Fcntl O_XXX constants available for importing
    require Fcntl;
    my @O = grep /^O_/, @Fcntl::EXPORT;
    Fcntl->import(@O);  # first we import what we want to export
    push(@EXPORT, @O);
};

################################################
## Constructor
##

sub new {
    my $type = shift;
    my $class = ref($type) || $type || "IO::File";
    @_ >= 0 && @_ <= 3
	or croak "usage: new $class [FILENAME [,MODE [,PERMS]]]";
    my $fh = $class->SUPER::new();
    if (@_) {
	$fh->open(@_)
	    or return undef;
    }
    $fh;
}

################################################
## Open
##

sub open {
    @_ >= 2 && @_ <= 4 or croak 'usage: $fh->open(FILENAME [,MODE [,PERMS]])';
    my ($fh, $file) = @_;
    if (@_ > 2) {
	my ($mode, $perms) = @_[2, 3];
	if ($mode =~ /^\d+$/) {
	    defined $perms or $perms = 0666;
	    return sysopen($fh, $file, $mode, $perms);
	} elsif ($mode =~ /:/) {
	    return open($fh, $mode, $file) if @_ == 3;
	    croak 'usage: $fh->open(FILENAME, IOLAYERS)';
	}
	if (defined($file) && length($file)
	    && ! File::Spec->file_name_is_absolute($file))
	{
	    $file = File::Spec->catfile(File::Spec->curdir(),$file);
	}
	$file = IO::Handle::_open_mode_string($mode) . " $file\0";
    }
    open($fh, $file);
}

1;
