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

public class AddNodeI extends AddNode
{
    public
    AddNodeI()
    {
    }

    public
    AddNodeI(Node left, Node right)
    {
	_left = left;
	_right = right;
    }

    public int
    calc(Ice.Current current)
    {
	return _left.calc(current) + _right.calc(current);
    }
}
