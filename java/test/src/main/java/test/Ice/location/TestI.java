// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.location.Test.HelloPrx;
import test.Ice.location.Test.TestIntf;

public class TestI implements TestIntf {
    TestI(
            ObjectAdapter adapter1,
            ObjectAdapter adapter2,
            ServerLocatorRegistry registry) {
        _adapter1 = adapter1;
        _adapter2 = adapter2;
        _registry = registry;

        _registry.addObject(
            _adapter1.add(new HelloI(), Util.stringToIdentity("hello")), null);
    }

    @Override
    public void shutdown(Current current) {
        _adapter1.getCommunicator().shutdown();
    }

    @Override
    public HelloPrx getHello(Current current) {
        return HelloPrx.uncheckedCast(
            _adapter1.createIndirectProxy(Util.stringToIdentity("hello")));
    }

    @Override
    public HelloPrx getReplicatedHello(Current current) {
        return HelloPrx.uncheckedCast(
            _adapter1.createProxy(Util.stringToIdentity("hello")));
    }

    @Override
    public void migrateHello(Current current) {
        final Identity id = Util.stringToIdentity("hello");
        try {
            _registry.addObject(_adapter2.add(_adapter1.remove(id), id), null);
        } catch (NotRegisteredException ex) {
            _registry.addObject(_adapter1.add(_adapter2.remove(id), id), null);
        }
    }

    private final ServerLocatorRegistry _registry;
    private final ObjectAdapter _adapter1;
    private final ObjectAdapter _adapter2;
}
