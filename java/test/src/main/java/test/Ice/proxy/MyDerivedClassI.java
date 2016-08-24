// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.proxy;

import test.Ice.proxy.Test._MyDerivedClassDisp;

public final class MyDerivedClassI implements _MyDerivedClassDisp
{
    public MyDerivedClassI()
    {
    }

    @Override
    public com.zeroc.Ice.ObjectPrx echo(com.zeroc.Ice.ObjectPrx obj, com.zeroc.Ice.Current c)
    {
        return obj;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current c)
    {
        c.adapter.getCommunicator().shutdown();
    }

    @Override
    public java.util.Map<String, String> getContext(com.zeroc.Ice.Current current)
    {
        return _ctx;
    }

    @Override
    public boolean ice_isA(String s, com.zeroc.Ice.Current current)
    {
        _ctx = current.ctx;
        return _MyDerivedClassDisp.super.ice_isA(s, current);
    }

    private java.util.Map<String, String> _ctx;
}
