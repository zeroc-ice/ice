// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Complex;

public class NumberNodeI extends NumberNode
{
    public
    NumberNodeI()
    {
    }

    public
    NumberNodeI(int number)
    {
	_number = number;
    }

    public int
    calc(Ice.Current current)
    {
	return _number;
    }
}
