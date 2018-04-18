#!/usr/bin/perl
#


use warnings;
use strict;
use IPC::Open3;


my @testcases = `ls testcases/*/*`;

my @debouncers = `ls ./debounce-*`;

map {chomp} @testcases;
map { chomp} @debouncers;

my $test_num =1;

my %stats_by_db ;
my %fails_by_test;
my %fails_by_db;
#run_debouncer('Source data', $debouncers[0], $datafile,'i');

for my $debouncer (@debouncers) {
for my $test (@testcases) {
	next unless (-f $test);
	next if ($test =~ /\.(raw|bak)$/);
	my $presses = -1;
	my $metadata = `grep  PRESSES: $test`;
	if ($metadata =~ /PRESSES:\s*(\d*)/ ){
		$presses = $1;
	}
	my $title = `grep TITLE: $test `;
	$title =~ s/^.*?TITLE://;
	my $sample_rate = `grep #SAMPLES-PER-SECOND: $test`;
	if ($sample_rate =~ /SAMPLES-PER-SECOND:\s*(\d*)/) {
		$sample_rate = $1;
	} else {
		$sample_rate = 625; # 1.6ms per sample
	}
	chomp($title);
	my ($count,$debug) = run_debouncer ($debouncer, $test, $sample_rate, 'c');
	if ($count == $presses) {
		$stats_by_db{$debouncer}{ok}++;
		print "ok ". $test_num++. "     - $debouncer $test saw $presses presses\n";
	} else {
		$stats_by_db{$debouncer}{not_ok}++;
		print "not ok ". $test_num++ ." - $debouncer $test saw $count presses but expected $presses\n";
		push @{$fails_by_test{$test}},$debouncer;
		push @{$fails_by_db{$debouncer}},$test;
	}
	print $debug;
}
}

for my $db (sort { $stats_by_db{$b}{ok} <=> $stats_by_db{$a}{ok}} keys %stats_by_db){
	printf("%-40s: OK %-4d FAIL %-4d\n", $db,($stats_by_db{$db}{ok}||0),($stats_by_db{$db}{not_ok}||0));
	printf("%-40s: Failed: %s\n",$db, join(", ",@{$fails_by_db{$db}})) if ($stats_by_db{$db}{not_ok}||0);
}

sub run_debouncer {
	my $debouncer = shift;
	my $data = shift;
	my $sample_rate = shift;
	my $arg = shift;

	my $stderr='';

	my @samples = resample($data, 2000/$sample_rate);
	open3(\*CHLD_IN, \*CHLD_OUT, \*CHLD_ERR, "$debouncer $arg") or die "open3() failed $!";

	for my $line (@samples) {
		print CHLD_IN $line;
	}	
	close(CHLD_IN);
	my $result =<CHLD_OUT>;
			chomp($result);
	while (my $line =<CHLD_OUT>) {
			$stderr .= $line;
	}
	
	return $result, $stderr;
}





my $downsample_averaging = 1;



sub resample {
	my $file = shift;
	my $output_ratio = shift;
	open(FILE, '<', $file);

my $output_counter = 0;
my $input_counter  = 0;
my $downsample_samples     = 0;
my $downsample_accumulator = 0;
	my @output;

while ( my $line = <FILE> ) {
    $line =~ s/\#.*$//g;
    $line =~ s/[^01]//g;

    for my $digit ( split( //, $line ) ) {
        $input_counter++;
        if ( $output_ratio > 1 ) {
            while ( $output_counter < ( $input_counter * $output_ratio ) ) {
                $output_counter++;
                push @output, $digit
                  . " # Input sample $input_counter. Output sample: "
                  . $input_counter * $output_ratio . " - "
                  . $output_counter . "\n";

            }

        }
        else {

            if ($downsample_averaging) {
                $downsample_samples++;
                $downsample_accumulator += $digit;
                if ( ( $input_counter * $output_ratio ) >=
                    ( $output_counter + 1 ) )
                {
                    $output_counter++;
                    push @output, (
                        ( $downsample_accumulator / $downsample_samples ) > 0.5
                        ? '1'
                        : '0' )
                      . " # Input sample $input_counter. ($digit ) Output sample: "
                      . $input_counter * $output_ratio . " - "
                      . $output_counter . "\n";
                    $downsample_samples     = 0;
                    $downsample_accumulator = 0;
                }
                else {
			#print STDERR "  # Input sample $input_counter Output sample " . $input_counter * $output_ratio . " DISCARDED\n";

                }

            }
            else {

                if ( ( $input_counter * $output_ratio ) >
                    ( $output_counter + 1 ) )
                {
                    $output_counter++;
                    push @output, $digit
                      . " # Input sample $input_counter. Output sample: "
                      . $input_counter * $output_ratio . " - "
                      . $output_counter . "\n";
                }
                else {
			#print STDERR "  # Input sample $input_counter Output sample " . $input_counter * $output_ratio . " DISCARDED\n";
                }
            }

        }

    }

}

close(FILE);
return @output;
};
