// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	this.left = left;
	this.right = right;
    }

    public int
    calc(Ice.Current current)
    {
	return left.calc(current) * right.calc(current);
    }
}
