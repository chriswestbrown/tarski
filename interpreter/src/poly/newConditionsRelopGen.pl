#!/usr/bin/perl

# This script generates a table that is used to compute
# the function specialDeduction.cpp:newConditionRelops.
# Documentation for newConditionRelops should make what
# this does a bit clearer.


$qepcadbase = "/home/wcbrown/qesource/bin";

@x = ('x^2<0','x<0','x=0','x<=0','x>0','x/=0','x>=0','x^2>=0');
@P = ('P^2<0','P<0','P=0','P<=0','P>0','P/=0','P>=0','P^2>=0');
@Q = ('Q^2<0','Q<0','Q=0','Q<=0','Q>0','Q/=0','Q>=0','Q^2>=0');
@c = ('x (P + b Q) >= 0','x (P + b Q) > 0','x (P + b Q) = 0');
@b = ('b<0','b=0','b>0');

print "static int ncrT[8][8][8][3] = {\n";

foreach $i (0 .. 7) {
  if ($i != 0) { } print "{";
  foreach $j (0 .. 7) {
    if ($j != 0) { print " ,"; } else { print " "; }  print "{";
    foreach $k (0 .. 7) {
      if ($k != 0) { print ","; } print "{";
#      print $x[$i] . " /\\ " . $P[$j] . " /\\ " . $Q[$k] . "  :  ";
      foreach $r (0..2) {
	if ($r != 0) { print ","; }
	$res = 0; @mask = (1,2,4);
	foreach $s (0..2) {
	  $input = "[]\n(b,x,P,Q)\n0\n(A b)(A x)(A P)(A Q)[\n"
	    . "[ " . $x[$i] . " /\\ " . $P[$j] . " /\\ " . $Q[$k] . " /\\ " . $b[$s] . " ]\n"
	      . " ==> " . $c[$r] . "\n].\nfinish\n";
	  open(FOO,"> sctmp"); print FOO $input; close(FOO);
	  $qeout = `$qepcadbase/qepcad < sctmp | egrep 'TRUE|FALSE'`;
#	  print $input,$qeout,"\n\n";
	  if ($qeout =~ /TRUE/) { $res += $mask[$s]; }
	}
	print $res;
      }
      print "}";
    }
    print "} // ".@x[$i].",".@P[$j]."\n";
  }
  print "}"; if ($i != 7) {print ",";} print "\n";
}
print "};\n";

