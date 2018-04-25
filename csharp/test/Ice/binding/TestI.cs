// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class TestI : TestIntfDisp_
{
    public override string getAdapterName(Ice.Current current)
    {
        return current.adapter.getName();
    }
}
