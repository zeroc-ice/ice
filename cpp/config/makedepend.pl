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

open(OUT, "> .depend") || die "can't open .depend: $!";

while(($key,$value) = each %map)
{
    print OUT "$key$value\n";
}

close(OUT);
