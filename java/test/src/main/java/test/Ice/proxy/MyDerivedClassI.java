// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.proxy.Test.MyDerivedClass;

import java.util.Map;

public final class MyDerivedClassI implements MyDerivedClass {
    public MyDerivedClassI() {
    }

    @Override
    public ObjectPrx echo(ObjectPrx obj, Current c) {
        return obj;
    }

    @Override
    public void shutdown(Current c) {
        c.adapter.getCommunicator().shutdown();
    }

    @Override
    public Map<String, String> getContext(Current current) {
        return _ctx;
    }

    @Override
    public boolean ice_isA(String s, Current current) {
        _ctx = current.ctx;
        return MyDerivedClass.super.ice_isA(s, current);
    }

    private Map<String, String> _ctx;
}
