#!/usr/bin/perl

while(<>)
{
    if ($_ =~ /\(def ([^ ]*) (\[.*\])\)/)
    {
	$n = $1;
	$f = $2;
	chomp($f);
	#$s = "(def F $f)\n(def D (make-NuCADConjunction F))\n(display (msg D 'num-x-cells) \" \" (msg D 'num-t-cells) \" \" \"$n\" \"\\n\")\n";
	$s = "(def F $f)\n(def D (SAT-NuCADConjunction F))\n(display (msg (get D 1) 'num-x-cells) \" \" (head D) \" \" \"$n\" \"\\n\")\n";
	open(my $fh,'>','tmp.tin');
	print $fh $s;
	close($fh);
	print `tarski +N20000000 -t 60 -q tmp.tin`;
    }
}
