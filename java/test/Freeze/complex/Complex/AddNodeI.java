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

public class AddNodeI extends AddNode
{
    public
    AddNodeI()
    {
    }

    public
    AddNodeI(Node left, Node right)
    {
	this.left = left;
	this.right = right;
    }

    public int
    calc(Ice.Current current)
    {
	return left.calc(current) + right.calc(current);
    }
}
