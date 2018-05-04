#!/usr/bin/perl

$a = 1.0*$ARGV[0];
$b = 1.0*$ARGV[1];

if ($a == 0.0) { $a = 0.1; }
if ($b == 0.0) { $b = 0.1; }

$x = $b/($a + $b);

print $x."\n";


