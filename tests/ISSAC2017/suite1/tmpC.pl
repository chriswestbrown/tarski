#!/usr/bin/perl

while(<>)
{
    chomp;
    @A = split(/ /,$_);
    if ($A[8] == 0) { $r = "N/A"; }
    else 
    { 
	$x = $A[3]/$A[8]; 
	$r = sprintf("%.5f", $x);
    }
    print "$A[0] $A[1] $A[2] $A[3] $A[8] $A[9] $r\n";
}
