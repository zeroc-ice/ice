// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.proxy.AMD.Test.MyDerivedClass;

import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDMyDerivedClassI implements MyDerivedClass {
    public AMDMyDerivedClassI() {}

    @Override
    public CompletionStage<ObjectPrx> echoAsync(
            ObjectPrx obj, Current current) {
        return CompletableFuture.completedFuture(obj);
    }

    @Override
    public CompletionStage<Void> shutdownAsync(Current c) {
        c.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Map<String, String>> getContextAsync(
            Current current) {
        return CompletableFuture.completedFuture(_ctx);
    }

    @Override
    public boolean ice_isA(String s, Current current) {
        _ctx = current.ctx;
        return MyDerivedClass.super.ice_isA(s, current);
    }

    private Map<String, String> _ctx;
}
