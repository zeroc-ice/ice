// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;
import test.Ice.binding.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    TestI()
    {
    }

    @Override
    public String
    getAdapterName(Ice.Current current)
    {
        return current.adapter.getName();
    }
}
