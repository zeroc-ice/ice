// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public sealed class MyDerivedClassI : Test.MyDerivedClass
{
    public MyDerivedClassI()
    {
    }
    
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
    
    public override void opSleep(int duration, Ice.Current current)
    {
        System.Threading.Thread.Sleep(duration);
    }

    public override Ice.Context
    getContext(Ice.Current current)
    {
        return _ctx;
    }

    public override bool
    ice_isA(string s, Ice.Current current)
    {
        _ctx = current.ctx;
        return base.ice_isA(s, current);
    }

    private Ice.Context _ctx;
}
