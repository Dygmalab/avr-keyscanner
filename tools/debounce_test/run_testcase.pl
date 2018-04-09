#!/usr/bin/perl
#


use warnings;
use strict;

my $datafile = shift @ARGV;

my @debouncers = `ls ./debounce-*`;

map { chomp} @debouncers;

run_debouncer('Source data', $debouncers[0], $datafile,'i');

for my $debouncer (@debouncers) {
	my $title = $debouncer;
	$title =~ s/^..debounce-//;
	run_debouncer ($title, $debouncer, $datafile, '');
}

sub run_debouncer {
	my $title = shift;
	my $debouncer = shift;
	my $data = shift;
	my $arg = shift;
	printf("%-40s",$title);
	system("$debouncer $arg < $data");
}
