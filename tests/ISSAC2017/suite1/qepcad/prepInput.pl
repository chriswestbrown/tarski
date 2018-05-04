#!/usr/bin/perl

sub produceVars
{
    my @V = ("x","y","z","w");
    my $n = $_[0];
    my $r = $_[1];
    my $L = "($V[0]";
    for (my $i=1; $i < $n; $i++)
    {
	$L = $L.",".$V[$i];
    }
    $L = $L.")";
    return $L;
}

sub produceQList
{
    my @V = ("x","y","z","w");
    my $n = $_[0];
    my $r = $_[1];
    my $L = "";
    for (my $i=$r; $i < $n; $i++)
    {
	$L = $L."(F ".$V[$i].")";
    }
    return $L;
}

sub createQepcadInputFile{ 
    $m = @_[0];
    $n = @_[1];
    $r = @_[2];
    $i = @_[3];
    
    $ns = ($i < 10 ? "0" : "").$i;
    $comm1 =  "echo '(load \"../in.${m}.${n}.2\") (display F${ns} \"\\n\")' | tarski -CASSaclib -q 2> /dev/null | tail -n1";
    
    $F = `$comm1`;
    chomp $F;
    $VL = produceVars($n,$r);
    $QL = produceQList($n,$r);

    $filename = "qein.$m.$n.$r.$ns";

    print "Writing $filename ...\n";

    open(my $fh, '>', $filename) or die "Could not open file '$filename' $!";

    print $fh "[]\n${VL}\n";
    print $fh "$r\n";
    print $fh $QL.$F.".\n";
    print $fh "meas\n";
    print $fh "go\n";
    print $fh "go\n";
    print $fh "go\n";
    print $fh "d-stat\n";
    print $fh "quit\n";
    close $fh;

}


for(my $m = 4; $m <= 6; $m++)
{
    for(my $n = 2; $n <= 4; $n++)
    {
	for(my $r = 0; $r < $n; $r++)
	{
	    for(my $i = 1; $i <= 25; $i++)
	    {
		createQepcadInputFile($m,$n,$r,$i);
	    }
	}
    }
}
