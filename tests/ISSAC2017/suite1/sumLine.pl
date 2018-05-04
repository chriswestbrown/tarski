#!/usr/bin/perl

while(<>)
{
    chomp;
    my @nums = split / /, $_;
    $total = 0;
    foreach my $x (@nums) {
	$total += $x;
    }
    print $total."\n";
}
