// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.proxy;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.proxy.AMD.Test.MyDerivedClass;

public final class AMDMyDerivedClassI implements MyDerivedClass
{
    public AMDMyDerivedClassI()
    {
    }

    @Override
    public CompletionStage<com.zeroc.Ice.ObjectPrx> echoAsync(com.zeroc.Ice.ObjectPrx obj,
                                                              com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(obj);
    }

    @Override
    public CompletionStage<Void> shutdownAsync(com.zeroc.Ice.Current c)
    {
        c.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<java.util.Map<String, String>> getContextAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(_ctx);
    }

    @Override
    public boolean ice_isA(String s, com.zeroc.Ice.Current current)
    {
        _ctx = current.ctx;
        return MyDerivedClass.super.ice_isA(s, current);
    }

    private java.util.Map<String, String> _ctx;
}
