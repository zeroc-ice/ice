// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
