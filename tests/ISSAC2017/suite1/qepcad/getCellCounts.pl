#!/usr/bin/perl

for(my $n = 2; $n <= 4; $n++) {
    for(my $m = 4; $m <= 6; $m++) {
	for(my $r = 1; $r < $n; $r++) {

	    $total = 0.0;
	    $count = 0;
	    for(my $i = 1; $i <= 25; $i++)
	    { 
		$ns = ($i < 10 ? "0" : "").$i;
		
		$filename = "qeout.$m.$n.$r.$ns";
		open(my $fh, '<', $filename) or die "Could not open file '$filename' $!";
		
		$num = -1;
		while(<$fh>)
		{
		    chomp;
		    if ($_ =~ /Cells\s+(\S.*)/)
		    {
			@C = split(/\s+/,$1);
			$num = $C[$r-1];
			$total = $total + $num;
			$count++;
		    }
		}
		#print $num."\n";
	    }
	    $ave = ($total/$count)/2;
	    print "$m $n $r $ave $count\n";
	}
    }
}

