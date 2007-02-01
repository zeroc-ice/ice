// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class TestI extends _TestIntfDisp
{
    TestI()
    {
    }

    public String
    getAdapterName(Ice.Current current)
    {
        return current.adapter.getName();
    }
}
