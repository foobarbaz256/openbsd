# ex:ts=8 sw=4:
# $OpenBSD: Persistant.pm,v 1.2 2014/01/07 01:29:17 espie Exp $
#
# Copyright (c) 2003-2011 Marc Espie <espie@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use strict;
use warnings;

package OpenBSD::PackageRepository::Persistant;
our @ISA=qw(OpenBSD::PackageRepository::Distant);

our %distant = ();

sub may_exist
{
	my ($self, $name) = @_;
	my $l = $self->list;
	return grep {$_ eq $name } @$l;
}

sub grab_object
{
	my ($self, $object) = @_;

	my $cmdfh = $self->{cmdfh};
	my $getfh = $self->{getfh};

	print $cmdfh "ABORT\n";
	my $_;
	while (<$getfh>) {
		last if m/^ABORTED/o;
	}
	print $cmdfh "GET ", $self->{path}.$object->{name}.".tgz", "\n";
	close($cmdfh);
	$_ = <$getfh>;
	chomp;
	if (m/^ERROR:/o) {
		$self->{state}->fatal("transfer error: #1", $_);
	}
	if (m/^TRANSFER:\s+(\d+)/o) {
		my $buffsize = 10 * 1024;
		my $buffer;
		my $size = $1;
		my $remaining = $size;
		my $n;

		do {
			$n = read($getfh, $buffer,
				$remaining < $buffsize ? $remaining :$buffsize);
			if (!defined $n) {
				$self->{state}->fatal("Error reading: #1", $!);
			}
			$remaining -= $n;
			if ($n > 0) {
				syswrite STDOUT, $buffer;
			}
		} while ($n != 0 && $remaining != 0);
		exit(0);
	}
}

sub maxcount
{
	return 1;
}

sub opened
{
	my $self = $_[0];
	my $k = $self->{host};
	if (!defined $distant{$k}) {
		$distant{$k} = [];
	}
	return $distant{$k};
}

sub list
{
	my ($self) = @_;
	if (!defined $self->{list}) {
		if (!defined $self->{controller}) {
			$self->initiate;
		}
		my $cmdfh = $self->{cmdfh};
		my $getfh = $self->{getfh};
		my $path = $self->{path};
		my $l = [];
		print $cmdfh "LIST $path\n";
		my $_;
		$_ = <$getfh>;
		if (!defined $_) {
			$self->{state}->fatal("Could not initiate #1 session",
			    $self->urlscheme);
		}
		chomp;
		if (m/^ERROR:/o) {
			$self->{state}->fatal("#1", $_);
		}
		if (!m/^SUCCESS:/o) {
			$self->{state}->fatal("Synchronization error");
		}
		while (<$getfh>) {
			chomp;
			last if $_ eq '';
			push(@$l, $_);
		}
		$self->{list} = $l;
	}
	return $self->{list};
}

sub cleanup
{
	my $self = shift;
	if (defined $self->{controller}) {
		my $cmdfh = $self->{cmdfh};
		my $getfh = $self->{getfh};
		print $cmdfh "ABORT\nBYE\nBYE\n";
		CORE::close($cmdfh);
		CORE::close($getfh);
		waitpid($self->{controller}, 0);
	}
}

sub reinitialize
{
	my $self = shift;
	$self->initiate;
}

1;
