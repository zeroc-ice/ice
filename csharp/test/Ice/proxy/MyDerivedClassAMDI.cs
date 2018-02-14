// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

public sealed class MyDerivedClassI : Test.MyDerivedClass
{
    public MyDerivedClassI()
    {
    }
    
    public override void echo_async(Test.AMD_MyDerivedClass_echo cb,
                                    Ice.ObjectPrx obj,
                                    Ice.Current c)
    {
        cb.ice_response(obj);
    }

    public override void shutdown_async(Test.AMD_MyClass_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }
    
    public override void getContext_async(Test.AMD_MyClass_getContext cb, Ice.Current current)
    {
        cb.ice_response(_ctx);
    }

    public override bool ice_isA(string s, Ice.Current current)
    {
        _ctx = current.ctx;
        return base.ice_isA(s, current);
    }

    private Dictionary<string, string> _ctx;
}
