// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import test.Ice.binding.Test.TestIntf;

public class TestI implements TestIntf
{
    TestI()
    {
    }

    @Override
    public String getAdapterName(com.zeroc.Ice.Current current)
    {
        return current.adapter.getName();
    }
}
