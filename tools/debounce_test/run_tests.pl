#!/usr/bin/perl
#


use warnings;
use strict;

my @testcases = `ls testcases/*`;

my @debouncers = `ls ./debounce-*`;

map {chomp} @testcases;
map { chomp} @debouncers;

my $test_num =1;

my %stats_by_db ;
#run_debouncer('Source data', $debouncers[0], $datafile,'i');

for my $debouncer (@debouncers) {
for my $test (@testcases) {
	my $presses = -1;
	my $metadata = `grep  PRESSES: $test`;
	if ($metadata =~ /PRESSES:\s*(\d*)/ ){
		$presses = $1;
	}
	my $title = `grep TITLE: $test `;
	$title =~ s/^.*?TITLE://;
	chomp($title);
	my $count = run_debouncer ($title, $debouncer, $test, 'c');
	if ($count == $presses) {
		$stats_by_db{$debouncer}{ok}++;
		print "ok ". $test_num++. "     - $title $debouncer found $presses presses\n";
	} else {
		$stats_by_db{$debouncer}{not_ok}++;
		print "not ok ". $test_num++ ." - $title $debouncer found $count presses but expected $presses\n";
	}
}
}

for my $db (sort { $stats_by_db{$b}{ok} <=> $stats_by_db{$a}{ok}} keys %stats_by_db){
	printf("%-40s: OK %-4d FAIL %-4d\n", $db,($stats_by_db{$db}{ok}||0),($stats_by_db{$db}{not_ok}||0));
}

sub run_debouncer {
	my $title = shift;
	my $debouncer = shift;
	my $data = shift;
	my $arg = shift;
	my $result = `$debouncer $arg < $data`;
	chomp($result);
	return $result;
}
