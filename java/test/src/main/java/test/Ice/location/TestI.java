// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test.HelloPrx;
import test.Ice.location.Test.TestIntf;

public class TestI implements TestIntf
{
    TestI(com.zeroc.Ice.ObjectAdapter adapter1, com.zeroc.Ice.ObjectAdapter adapter2, ServerLocatorRegistry registry)
    {
        _adapter1 = adapter1;
        _adapter2 = adapter2;
        _registry = registry;

        _registry.addObject(_adapter1.add(new HelloI(), com.zeroc.Ice.Util.stringToIdentity("hello")), null);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _adapter1.getCommunicator().shutdown();
    }

    @Override
    public HelloPrx getHello(com.zeroc.Ice.Current current)
    {
        return HelloPrx.uncheckedCast(_adapter1.createIndirectProxy(com.zeroc.Ice.Util.stringToIdentity("hello")));
    }

    @Override
    public HelloPrx getReplicatedHello(com.zeroc.Ice.Current current)
    {
        return HelloPrx.uncheckedCast(_adapter1.createProxy(com.zeroc.Ice.Util.stringToIdentity("hello")));
    }

    @Override
    public void migrateHello(com.zeroc.Ice.Current current)
    {
        final com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("hello");
        try
        {
            _registry.addObject(_adapter2.add(_adapter1.remove(id), id), null);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
            _registry.addObject(_adapter1.add(_adapter2.remove(id), id), null);
        }
    }

    private ServerLocatorRegistry _registry;
    private com.zeroc.Ice.ObjectAdapter _adapter1;
    private com.zeroc.Ice.ObjectAdapter _adapter2;
}
