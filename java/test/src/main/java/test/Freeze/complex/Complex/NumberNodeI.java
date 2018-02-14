// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.complex.Complex;

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
