// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class MyDerivedClassI extends Test.MyDerivedClass
{
    public
    MyDerivedClassI()
    {
    }

    public void
    echo_async(Test.AMD_MyDerivedClass_echo cb,
                   Ice.ObjectPrx obj,
                   Ice.Current c)
    {
        cb.ice_response(obj);
    }

    public void
    shutdown_async(Test.AMD_MyClass_shutdown cb,
                   Ice.Current c)
    {
        c.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public void
    getContext_async(Test.AMD_MyClass_getContext cb, Ice.Current current)
    {
        cb.ice_response(_ctx);
    }

    public boolean
    ice_isA(String s, Ice.Current current)
    {
        _ctx = current.ctx;
        return super.ice_isA(s, current);
    }

    private java.util.Map _ctx;
}
