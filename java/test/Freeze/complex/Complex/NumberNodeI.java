// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
	this.number = number;
    }

    public int
    calc(Ice.Current current)
    {
	return number;
    }
}
