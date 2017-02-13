// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.proxy;
import test.Ice.proxy.AMD.Test.AMD_MyClass_getContext;
import test.Ice.proxy.AMD.Test.AMD_MyClass_shutdown;
import test.Ice.proxy.AMD.Test.AMD_MyDerivedClass_echo;
import test.Ice.proxy.AMD.Test.MyDerivedClass;

public final class AMDMyDerivedClassI extends MyDerivedClass
{
    public
    AMDMyDerivedClassI()
    {
    }

    @Override
    public void
    echo_async(AMD_MyDerivedClass_echo cb,
                   Ice.ObjectPrx obj,
                   Ice.Current c)
    {
        cb.ice_response(obj);
    }

    @Override
    public void
    shutdown_async(AMD_MyClass_shutdown cb,
                   Ice.Current c)
    {
        c.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    public void
    getContext_async(AMD_MyClass_getContext cb, Ice.Current current)
    {
        cb.ice_response(_ctx);
    }

    @Override
    public boolean
    ice_isA(String s, Ice.Current current)
    {
        _ctx = current.ctx;
        return super.ice_isA(s, current);
    }

    private java.util.Map<String, String> _ctx;
}
