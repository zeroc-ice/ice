// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Complex;

public class MultiplyNodeI extends MultiplyNode
{
    public
    MultiplyNodeI()
    {
    }

    public
    MultiplyNodeI(Node left, Node right)
    {
	_left = left;
	_right = right;
    }

    public int
    calc(Ice.Current current)
    {
	return _left.calc(current) * _right.calc(current);
    }
}
