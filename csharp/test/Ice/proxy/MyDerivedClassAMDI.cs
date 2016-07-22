// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
using System.Collections.Generic;

public sealed class MyDerivedClassI : Test.MyDerivedClass
{
    public MyDerivedClassI()
    {
    }
    
    public override void
    echoAsync(Ice.ObjectPrx obj, Action<Ice.ObjectPrx> response, Action<Exception> exception, Ice.Current c)
    {
        response(obj);
    }

    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }
    
    public override void
    getContextAsync(Action<Dictionary<string, string>> response, Action<Exception> exception, Ice.Current current)
    {
        response(_ctx);
    }

    public override bool ice_isA(string s, Ice.Current current)
    {
        _ctx = current.ctx;
        return base.ice_isA(s, current);
    }

    private Dictionary<string, string> _ctx;
}
