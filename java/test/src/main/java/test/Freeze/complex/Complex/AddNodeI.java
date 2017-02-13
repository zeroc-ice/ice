// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.complex.Complex;

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
