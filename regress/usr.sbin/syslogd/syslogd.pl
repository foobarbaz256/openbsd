#!/usr/bin/perl
#	$OpenBSD: syslogd.pl,v 1.5 2015/02/13 21:40:50 bluhm Exp $

# Copyright (c) 2010-2014 Alexander Bluhm <bluhm@openbsd.org>
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
use Socket;
use Socket6;

use Client;
use Syslogd;
use Server;
use Syslogc;
use RSyslogd;
require 'funcs.pl';

sub usage {
	die "usage: syslogd.pl [test-args.pl]\n";
}

my $testfile;
our %args;
if (@ARGV and -f $ARGV[-1]) {
	$testfile = pop;
	do $testfile
	    or die "Do test file $testfile failed: ", $@ || $!;
}
@ARGV == 0 or usage();

if ($args{rsyslogd}) {
	$args{rsyslogd}{listen}{domain} ||= AF_INET;
	$args{rsyslogd}{listen}{addr}   ||= "127.0.0.1";
	$args{rsyslogd}{listen}{proto}  ||= "udp";
}
foreach my $name (qw(client syslogd server rsyslogd)) {
	$args{$name} or next;
	foreach my $action (qw(connect listen)) {
		my $h = $args{$name}{$action} or next;
		foreach my $k (qw(domain addr proto port)) {
			$args{$name}{"$action$k"} = $h->{$k};
		}
	}
}
my($s, $c, $r, @m);
$s = RSyslogd->new(
    %{$args{rsyslogd}},
    listenport          => scalar find_ports(%{$args{rsyslogd}{listen}}),
    testfile            => $testfile,
) if $args{rsyslogd};
$s ||= Server->new(
    func                => \&read_log,
    listendomain        => AF_INET,
    listenaddr          => "127.0.0.1",
    %{$args{server}},
    testfile            => $testfile,
    client              => \$c,
    syslogd             => \$r,
) unless $args{server}{noserver};
$args{syslogc} = [ $args{syslogc} ] if ref $args{syslogc} eq 'HASH';
my $i = 0;
@m = map { Syslogc->new(
    %{$_},
    testfile            => $testfile,
    ktracefile          => "syslogc-$i.ktrace",
    logfile             => "syslogc-".$i++.".log",
) } @{$args{syslogc}};
$r = Syslogd->new(
    connectaddr         => "127.0.0.1",
    connectport         => $s && $s->{listenport},
    ctlsock		=> @m && $m[0]->{ctlsock},
    %{$args{syslogd}},
    testfile            => $testfile,
    client              => \$c,
    server              => \$s,
);
$c = Client->new(
    func                => \&write_log,
    %{$args{client}},
    testfile            => $testfile,
    syslogd             => \$r,
    server              => \$s,
) unless $args{client}{noclient};

$r->run unless $r->{late};
$s->run->up unless $args{server}{noserver};
$r->run if $r->{late};
$r->up;
my $control = 0;
foreach (@m) {
	if ($_->{early} || $_->{stop}) {
		$_->run->up;
		$control++;
	}
}
$r->loggrep("Accepting control connection") if $control;
foreach (@m) {
	if ($_->{stop}) {
		$_->kill('STOP');
	}
}
$c->run->up unless $args{client}{noclient};

$c->down unless $args{client}{noclient};
$s->down unless $args{server}{noserver};
foreach (@m) {
	if ($_->{stop}) {
		$_->kill('CONT');
		$_->down;
	} elsif ($_->{early}) {
		$_->down;
	} else {
		$_->run->up->down;
	}
}
$r->kill_child;
$r->down;

$args{check}->({client => $c, syslogd => $r, server => $s}) if $args{check};
check_logs($c, $r, $s, \@m, %args);
