#!/usr/bin/perl
##############################################################
# This script creates the new tarski.h file from the old.  It
# increments the minor version number and gives a new time/date
# stamp.  I know ... it's not good perl code!
##############################################################
while(<>)
{
    if ($_ =~ /tarskiVersion\s*=\s*\"(\d*)\.(-?\d*)\"/)
    {
	$majv = $1;
	$minv = $2;
	$minv_new = $minv + 1;
	print("static const char * tarskiVersion     = \"${majv}.${minv_new}\";\n");
    }
    else
    {
	if ($_ =~ /tarskiVersionDate\s*=/)
	{
	    $date_new = `date`;
	    chop $date_new;
	    print("static const char * tarskiVersionDate = \"${date_new}\";\n");
	}
	else
	{
	    print($_);
	}
    }
}

