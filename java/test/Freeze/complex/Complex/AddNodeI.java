// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
