#!/bin/perl

while(<STDIN>)
{
    if(/\\$/)
    {
	chop;
	chop;
	$line = $line . $_;
    }
    else
    {
	$_ = $line . $_;
	
	($t, @d) = split;

	foreach (@d)
	{
	    if(!/^\//)
	    {
		$map{$t} .= " $_";
	    }
	}

	$line = ""
    }
}

open(UNIX, "> .depend") || die "can't open .depend: $!";
open(WIN, "> .windep") || die "can't open .windep: $!";

while(($key,$value) = each %map)
{
    print UNIX "$key$value\n";
    $key =~ s/\.o/.obj/;
    $value =~ s/\//\\/g;
    print WIN "$key$value\n";
}

close(UNIX);
close(WIN);
