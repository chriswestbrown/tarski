#!/usr/bin/perl

#########################################################
# This precommit script should be run prior to a commit
# that's intended to be pushed to the repo.  It incremements
# the version number and puts a new version date stamp on.
#########################################################

sub bumpVersion {
    my $fname = $_[0];
    my $pre = 'static const char\* version = "';
    my $post = '";';
    open(my $fh, '<', $fname)
	or die "Could not open file '$fname' $!";
    while (my $row = <$fh>) {
	chomp $row;
	if ($row =~ /^($pre)((\d+)[.](\d+))($post)$/) {
	    my $vers = $2;
	    my $versMaj = $3;
	    my $versMin = $4;
	    my $versMinNew = $versMin + 1;
	    my $versNew = "$versMaj.$versMinNew";
	    my $pat = "s/$pre$vers$post/$pre$versNew$post/;";
	    my $comm = "sed -i '$pat' $fname";
	    `$comm`;
	    return $versNew;
	}
    }
    return "ERROR";
}

sub stampVersionDate {
    my $fname = $_[0];
    $pre = 'static const char\* versdate = "';
    $post = '";';
    open(my $fh, '<', $fname)
	or die "Could not open file '$fname' $!";
    while (my $row = <$fh>) {
	chomp $row;
	if ($row =~ /^($pre)(.*)($post)$/) {
	    $oldDate = $2;
	    $dateNew = `date`;
	    chomp $dateNew;
	    $pat = "s/$pre$oldDate$post/$pre$dateNew$post/;";
	    $comm = "sed -i '$pat' $fname";
	    print $comm."\n";
	    `$comm`;
	    return $dateNew;
	}
    }
    return "ERROR";
}

# qepcad proper
my $f1 = "../source/main/PRINTBANNER.c";
$versNEW = bumpVersion($f1);
print "Bumped version number to $versNEW.\n";    
$dateNEW = stampVersionDate($f1);
print "Stamped version date to $dateNEW.\n";

# cad2d
my $f2 = "../cad2d/src/PRINTCAD2DBANNER.c";
$versNEW = bumpVersion($f2);
print "Bumped version number to $versNEW.\n";    
$dateNEW = stampVersionDate($f2);
print "Stamped version date to $dateNEW.\n";




