// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import test.Ice.ami.Test.CloseMode;
import test.Ice.ami.Test.Outer.Inner.TestIntf;

public class TestII implements TestIntf
{
    public OpResult op(int i, com.zeroc.Ice.Current current)
    {
        OpResult result = new OpResult();
        result.returnValue = i;
        result.j = i;
        return result;
    }
}
