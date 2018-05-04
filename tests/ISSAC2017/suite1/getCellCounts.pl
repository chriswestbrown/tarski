#!/usr/bin/perl

for(my $n = 2; $n <= 4; $n++) {
    for(my $m = 4; $m <= 6; $m++) {
	for(my $r = 1; $r < $n; $r++) {
	    
	    $total = 0.0;
	    $count = 0;
	    
	    $filename = "res.$m.$n.r$r";
	    open(my $fh, '<', $filename) or die "Could not open file '$filename' $!";
	    while(<$fh>)
	    {
		chomp;
		@C = split(/\s+/,$_);
		$num = $C[7];
		$total = $total + $num;
		$count++;
	    }
	    #print $num."\n";
	    $ave = $total/$count;
	    print "$m $n $r $ave $count\n";
	}
    }
}


