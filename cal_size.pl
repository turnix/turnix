#!/usr/bin/env perl
#

use strict;
use warnings;

my $name_off;
my %sections = (
	".text" => 1,
	".data" => 1,
	".rodata" => 1,
	".bss" => 1
);
my @slices;
my $total_size = 0;
my $size;

while (<>) {
	if (/\[Nr\]/) {
		$name_off = index($_, "Name");
	} elsif (/\[[\s0-9]+\]/) {
		@slices = split(/\s+/, substr($_, $name_off));
		if ($sections{$slices[0]}) {
			$size = hex($slices[4]);
			$total_size += $size;
			print "$slices[0]\t$size\n";
		}
	}
}

print "total\t$total_size\n";
