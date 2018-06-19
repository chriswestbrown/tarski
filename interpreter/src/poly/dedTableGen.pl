#!/usr/bin/perl

# This script generates two tables: dedTableGE and dedTableLE.
# Consider the expression P + x*Q.  Suppose we know Q sigmaQ 0
# holds, we know an interval [a,b] (wherer s_a and s_b are
# the signs of a & b), where for any x in [a,b] P + x*Q >= 0.
# Finally let c_strict and c_equal be masks describing the
# parts of the interval (left-endpoint, interior, right-endpoint)
# the inequality holds strictly and with equality, respectively.
# Then: dedTableGE[sigmaQ][s_a+1][s_b+1][c_strict][c_equal] is
# a relop sigmaP such that we may conclude that P sigmaP 0 holds.
#
# dedTableLE is the same thing for P + x*Q <= 0.

$qepcadbase = "/home/wcbrown/qesource/bin";

@Q = ('Q^2<0','Q<0','Q=0','Q<=0','Q>0','Q/=0','Q>=0','Q^2>=0');
@a = ('a<0','a=0','a>0');
@b = ('b<0','b=0','b>0');
@I = ('1=0','a=x','a<x/\\x<b','a<=x/\\x<b','x=b','[a=x\\/x=b]','a<x/\\x<=b','a<=x/\\x<=b');
@P = ('P<0','P=0','P>0');

print "static const char dedTableGE[8][3][3][8][8] = {\n";

foreach $i (0 .. 7) {
  if ($i != 0) { print ",\n"; }
  print "{\n";
  foreach $j (0 .. 2) {
    if ($j != 0) { print ",\n"; }
    print "{\n";
    foreach $k (0 .. 2) {
      if ($k != 0) { print ",\n"; }
      print "{\n";
      foreach $r (0..7) {
	if ($r != 0) { print ",\n"; }
	print "{\n";
	foreach $s (0..7) {
	  if ($s != 0) { print ","; } else { print " "; }


	  $res = 0; @mask = (1,2,4);
	  foreach $t (0..2) {
	    $qein = "[]\n(P,Q,a,b,x)\n0\n(E P)(E Q)(E a)(E b)(A x)[\n".
	      "[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ a <= x /\\ x <= b ] ==> P + x Q >= 0 ] ]"."/\\\n".
		"[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ $I[$r] ] ==> P + x Q > 0 ] ]"."/\\\n".
		  "[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ $I[$s] ] ==> P + x Q = 0 ] ]"."/\\\n".
		    "$P[$t]\n].\nfinish\n";
	    open(FOO,"> sctmp"); print FOO $qein; close(FOO);
	    $qeout = `$qepcadbase/qepcad < sctmp | egrep 'TRUE|FALSE'`;
	    if ($qeout =~ /TRUE/) { $res += $mask[$t]; }
	    
	    #print $qein, "\n";
	    #print $qeout, "\n";
	  }
	  print "$res // $Q[$i], $a[$j], $b[$k], $r, $s, --> $res = T[$i][$j][$k][$r][$s]\n";
	}
	print "}";
      }
      print "}";
    }
    print "}";
  }
  print "}";
}
print "};\n";


print "static const char dedTableLE[8][3][3][8][8] = {\n";

foreach $i (0 .. 7) {
  if ($i != 0) { print ",\n"; }
  print "{\n";
  foreach $j (0 .. 2) {
    if ($j != 0) { print ",\n"; }
    print "{\n";
    foreach $k (0 .. 2) {
      if ($k != 0) { print ",\n"; }
      print "{\n";
      foreach $r (0..7) {
	if ($r != 0) { print ",\n"; }
	print "{\n";
	foreach $s (0..7) {
	  if ($s != 0) { print ","; } else { print " "; }


	  $res = 0; @mask = (1,2,4);
	  foreach $t (0..2) {
	    $qein = "[]\n(P,Q,a,b,x)\n0\n(E P)(E Q)(E a)(E b)(A x)[\n".
	      "[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ a <= x /\\ x <= b ] ==> P + x Q <= 0 ] ]"."/\\\n".
		"[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ $I[$r] ] ==> P + x Q < 0 ] ]"."/\\\n".
		  "[ $Q[$i] /\\ $a[$j] /\\ a <= b /\\ $b[$k] /\\ [ [ $I[$s] ] ==> P + x Q = 0 ] ]"."/\\\n".
		    "$P[$t]\n].\nfinish\n";
	    open(FOO,"> sctmp"); print FOO $qein; close(FOO);
	    $qeout = `$qepcadbase/qepcad < sctmp | egrep 'TRUE|FALSE'`;
	    if ($qeout =~ /TRUE/) { $res += $mask[$t]; }
	    
	    #print $qein, "\n";
	    #print $qeout, "\n";
	  }
	  print "$res // $Q[$i], $a[$j], $b[$k], $r, $s, --> $res = T[$i][$j][$k][$r][$s]\n";
	}
	print "}";
      }
      print "}";
    }
    print "}";
  }
  print "}";
}
print "};\n";


