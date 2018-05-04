#!/usr/bin/perl

use Math::BigFloat ':constant';

$count = 0;
$numpolys = $ARGV[0];
$numvars = $ARGV[1];
$r = $ARGV[2];
$filename = "in.$numpolys.$numvars.2";
open(my $fhin, '<', $filename) or die "Could not open file '$filename' $!";
@var = ("x","y","z","w","u","v");
$varstring = "(";
$point = "(";
for (my $i=0; $i < $numvars; $i++) 
{
    $x = $var[$i];
    $varstring = "$varstring $x";
    $point = "$point 0";
}
$varstring = $varstring.")";
$point = "$point )";

$totalCellsFull = 0;
$totalCellsProj = 0;
while(<$fhin>)
{
    $pin = $_;
    chomp $pin;
    if ($pin =~ /^;/) { next; }
   $pin =~ /[^ ]+ ([^ ]+) /;
    $fname = $1;

#    for (my $r = 0; $r < $numvars; $r++)
    {
	if ($r == 0)
	{
	    $comm = "$pin ".
		"(def t0 (cputime)) ".
		"(def D (SAT-NuCADConjunction '$varstring '$point $fname)) ".
		"(def t1 (cputime)) ".
		"(display (- t1 t0) \" \" -1 \" \" (msg (head (tail D)) 'num-x-cells) \" \" -1)";
	    my $filename = 'tmp.tin';
	    open(my $fh, '>', $filename) or die "Could not open file '$filename' $!";
	    print $fh $comm;
	    close $fh;
	    $res = `tarski +N20000000 -q tmp.tin 2> /dev/null`;
	    print "$numpolys $numvars $r $fname $res\n";

	}
	else
	{
	    $comm = "$pin ".
		"(def t0 (cputime)) ".
		"(def D (make-NuCADConjunction '$varstring '$point $fname)) ".
		"(def nx (msg D 'num-x-cells)) ".
		"(def t1 (cputime)) ".
		"(msg D 'project $r) ".
		"(def t2 (cputime)) ".
		"(display (- t1 t0) \" \" (- t2 t1) \" \" nx \" \" (msg D 'num-x-cells) )";
		
	    
	    my $filename = 'tmp.tin';
	    open(my $fh, '>', $filename) or die "Could not open file '$filename' $!";
	    print $fh $comm;
	    close $fh;
	    $res = `tarski +N20000000 -q tmp.tin 2> /dev/null`;
	    print "$numpolys $numvars $r $fname $res\n";
	}
    }
}


