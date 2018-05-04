#!/usr/bin/perl


for(my $n = 2; $n <= 4; $n++)
{
    for(my $m = 4; $m <= 6; $m++)
    {
	for(my $r = 0; $r < $n; $r++)
	{
	    for(my $i = 1; $i <= 25; $i++)
	    {
		my $ns = ($i < 10 ? "0" : "").$i;
		$infilename = "qein.$m.$n.$r.$ns";
		$outfilename = "qeout.$m.$n.$r.$ns";		
		$N = 10000000;
		if ($n == 3) { $N = 100000000; }
		if ($n == 4) { $N = 1000000000; }		
		$comm = "qepcad +N${N} < $infilename > $outfilename";		
		print $comm."\n";	
#		`$comm`;
	    }
	}
    }
}
