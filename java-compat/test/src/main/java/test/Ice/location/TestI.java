// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test.HelloPrx;
import test.Ice.location.Test.HelloPrxHelper;
import test.Ice.location.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    TestI(Ice.ObjectAdapter adapter1, Ice.ObjectAdapter adapter2, ServerLocatorRegistry registry)
    {
        _adapter1 = adapter1;
        _adapter2 = adapter2;
        _registry = registry;

        _registry.addObject(_adapter1.add(new HelloI(), Ice.Util.stringToIdentity("hello")));
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _adapter1.getCommunicator().shutdown();
    }

    @Override
    public HelloPrx
    getHello(Ice.Current current)
    {
        return HelloPrxHelper.uncheckedCast(_adapter1.createIndirectProxy(Ice.Util.stringToIdentity("hello")));
    }

    @Override
    public HelloPrx
    getReplicatedHello(Ice.Current current)
    {
        return HelloPrxHelper.uncheckedCast(_adapter1.createProxy(Ice.Util.stringToIdentity("hello")));
    }

    @Override
    public void
    migrateHello(Ice.Current current)
    {
        final Ice.Identity id = Ice.Util.stringToIdentity("hello");
        try
        {
            _registry.addObject(_adapter2.add(_adapter1.remove(id), id));
        }
        catch(Ice.NotRegisteredException ex)
        {
            _registry.addObject(_adapter1.add(_adapter2.remove(id), id));
        }
    }

    private ServerLocatorRegistry _registry;
    private Ice.ObjectAdapter _adapter1;
    private Ice.ObjectAdapter _adapter2;
}
