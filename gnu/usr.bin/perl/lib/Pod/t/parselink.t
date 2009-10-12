#!/usr/bin/perl -w
#
# parselink.t -- Tests for Pod::ParseLink.
#
# Copyright 2001 by Russ Allbery <rra@stanford.edu>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.

# The format of each entry in this array is the L<> text followed by the
# five-element parse returned by parselink.  When adding a new test, also
# increment the test count in the BEGIN block below.  We don't use any of the
# fancy test modules intentionally for backward compatibility to older
# versions of Perl.
@TESTS = (
    [ 'foo',
      undef, 'foo', 'foo', undef, 'pod' ],

    [ 'foo|bar',
      'foo', 'foo', 'bar', undef, 'pod' ],

    [ 'foo/bar',
      undef, '"bar" in foo', 'foo', 'bar', 'pod' ],

    [ 'foo/"baz boo"',
      undef, '"baz boo" in foo', 'foo', 'baz boo', 'pod' ],

    [ '/bar',
      undef, '"bar"', undef, 'bar', 'pod' ],

    [ '/"baz boo"',
      undef, '"baz boo"', undef, 'baz boo', 'pod' ],

    [ '/baz boo',
      undef, '"baz boo"', undef, 'baz boo', 'pod' ],

    [ 'foo bar/baz boo',
      undef, '"baz boo" in foo bar', 'foo bar', 'baz boo', 'pod' ],

    [ 'foo bar  /  baz boo',
      undef, '"baz boo" in foo bar', 'foo bar', 'baz boo', 'pod' ],

    [ "foo\nbar\nbaz\n/\nboo",
      undef, '"boo" in foo bar baz', 'foo bar baz', 'boo', 'pod' ],

    [ 'anchor|name/section',
      'anchor', 'anchor', 'name', 'section', 'pod' ],

    [ '"boo var baz"',
      undef, '"boo var baz"', undef, 'boo var baz', 'pod' ],

    [ 'bar baz',
      undef, '"bar baz"', undef, 'bar baz', 'pod' ],

    [ '"boo bar baz / baz boo"',
      undef, '"boo bar baz / baz boo"', undef, 'boo bar baz / baz boo',
      'pod' ],

    [ 'fooZ<>bar',
      undef, 'fooZ<>bar', 'fooZ<>bar', undef, 'pod' ],

    [ 'Testing I<italics>|foo/bar',
      'Testing I<italics>', 'Testing I<italics>', 'foo', 'bar', 'pod' ],

    [ 'foo/I<Italic> text',
      undef, '"I<Italic> text" in foo', 'foo', 'I<Italic> text', 'pod' ],

    [ 'fooE<verbar>barZ<>/Section C<with> I<B<other> markup',
      undef, '"Section C<with> I<B<other> markup" in fooE<verbar>barZ<>',
      'fooE<verbar>barZ<>', 'Section C<with> I<B<other> markup', 'pod' ],

    [ 'Nested L<http://www.perl.org/>|fooE<sol>bar',
      'Nested L<http://www.perl.org/>', 'Nested L<http://www.perl.org/>',
      'fooE<sol>bar', undef, 'pod' ],

    [ 'ls(1)',
      undef, 'ls(1)', 'ls(1)', undef, 'man' ],

    [ '  perlfunc(1)/open  ',
      undef, '"open" in perlfunc(1)', 'perlfunc(1)', 'open', 'man' ],

    [ 'some manual page|perl(1)',
      'some manual page', 'some manual page', 'perl(1)', undef, 'man' ],

    [ 'http://www.perl.org/',
      undef, 'http://www.perl.org/', 'http://www.perl.org/', undef, 'url' ],

    [ 'news:yld72axzc8.fsf@windlord.stanford.edu',
      undef, 'news:yld72axzc8.fsf@windlord.stanford.edu',
      'news:yld72axzc8.fsf@windlord.stanford.edu', undef, 'url' ]
);

BEGIN {
    chdir 't' if -d 't';
    unshift (@INC, '../blib/lib');
    $| = 1;
    print "1..25\n";
}

END {
    print "not ok 1\n" unless $loaded;
}

use Pod::ParseLink;
$loaded = 1;
print "ok 1\n";

# Used for reporting test failures.
my @names = qw(text inferred name section type);

my $n = 2;
for (@TESTS) {
    my @expected = @$_;
    my $link = shift @expected;
    my @results = parselink ($link);
    my $okay = 1;
    for (0..4) {
        # Make sure to check undef explicitly; we don't want undef to match
        # the empty string because they're semantically different.
        unless ((!defined ($results[$_]) && !defined ($expected[$_]))
                || (defined ($results[$_]) && defined ($expected[$_])
                    && $results[$_] eq $expected[$_])) {
            print "not ok $n\n" if $okay;
            print "# Incorrect $names[$_]:\n";
            print "#   expected: $expected[$_]\n";
            print "#       seen: $results[$_]\n";
            $okay = 0;
        }
    }
    print "ok $n\n" if $okay;
    $n++;
}
