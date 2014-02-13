#!/usr/bin/perl

# Use this to estimate the size of things in the linker table.

$thisaddr = 0;
$lastsize = 0;
$lastaddr = 0;
$lasttag = "";
$lastname = "";

# The interesting part is that we want to print the last items. 

while(<>) {

    chomp;
    @parts = split(/ /);

    $thisaddr = hex $parts[0];
    $entrylen = $thisaddr - $lastaddr;

    printf("%08x %6x %s %s\n",$lastaddr,$entrylen, $lasttag, $lastname);

    $lastaddr = $thisaddr;
    $lasttag  = $parts[1];
    $lastname = $parts[2];


    
 }
