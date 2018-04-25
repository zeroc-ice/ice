// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.proxy;
import test.Ice.proxy.Test._MyDerivedClassDisp;

public final class MyDerivedClassI extends _MyDerivedClassDisp
{
    public
    MyDerivedClassI()
    {
    }

    @Override
    public Ice.ObjectPrx
    echo(Ice.ObjectPrx obj, Ice.Current c)
    {
        return obj;
    }

    @Override
    public void
    shutdown(Ice.Current c)
    {
        c.adapter.getCommunicator().shutdown();
    }

    @Override
    public java.util.Map<String, String>
    getContext(Ice.Current current)
    {
        return _ctx;
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
