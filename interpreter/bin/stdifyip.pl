#!/usr/bin/perl

$N = scalar @ARGV;
$i = 0;
while($i < $N)
{
    $name = $ARGV[$i];
    `stdify.pl $name > $name.stdify`;
    `mv -f $name.stdify $name`;
    $i++;
}
