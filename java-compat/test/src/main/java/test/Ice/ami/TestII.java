// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.ami;

import test.Ice.ami.Test.Outer.Inner._TestIntfDisp;

public class TestII extends _TestIntfDisp
{
    public int op(int i, Ice.IntHolder j, Ice.Current current)
    {
        j.value = i;
        return i;
    }
}
