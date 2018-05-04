#!/usr/bin/perl

my $filename = $ARGV[0];
open(my $fh, '<:encoding(UTF-8)', $filename)
  or die "Could not open file '$filename' $!";

$unsFound = 0;
while (my $line = <$fh>) {    
    if ($unsFound == 0 && $line =~ /\s*using namespace std/) { $unsFound++; next; }
    if ($unsFound > 0)
    {
	$line =~ s/\b(ostringstream|istringstream|string|ostream|istream|cout|cerr|cin|endl|vector|map|set|stack|streambuf|bitset)\b/std::$1/g;
    }
    print $line; 
}
