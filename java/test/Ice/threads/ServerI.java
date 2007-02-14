// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class ServerI extends Test._ServerDisp
{
    public
    ServerI(Ice.Communicator communicator)
    {
        _hook = new ThreadHookI();
        _adapters = new Test.AdapterPrx[4];

        Ice.Communicator comm;
        Ice.ObjectAdapter adapter;
        Ice.InitializationData id = new Ice.InitializationData();
        Test.AdapterPrx proxy;

        id.threadHook = _hook;

        Ice.Identity ident;

        //
        // Create an adapter that uses the communicator's thread pool.
        //
        id.properties = communicator.getProperties()._clone();
        id.properties.setProperty("Ice.ThreadPerConnection", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "2");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter1", "default");
        ident = comm.stringToIdentity("adapter1");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, false), ident));
        adapter.activate();
        _adapters[0] = proxy;

        //
        // Create an adapter that uses the communicator's default setting for thread-per-connection.
        //
        id.properties = communicator.getProperties()._clone();
        id.properties.setProperty("Ice.ThreadPerConnection", "1");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter2", "default");
        ident = comm.stringToIdentity("adapter2");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, true), ident));
        adapter.activate();
        _adapters[1] = proxy;

        //
        // Create an adapter that uses a private thread pool.
        //
        id.properties = communicator.getProperties()._clone();
        id.properties.setProperty("Ice.ThreadPerConnection", "1");
        id.properties.setProperty("Adapter3.ThreadPool.SizeMax", "2");
        id.properties.setProperty("Adapter3.ThreadPool.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter3", "default");
        ident = comm.stringToIdentity("adapter3");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, false), ident));
        adapter.activate();
        _adapters[2] = proxy;

        //
        // Create an adapter that uses an explicit setting for thread-per-connection.
        //
        id.properties = communicator.getProperties()._clone();
        id.properties.setProperty("Ice.ThreadPerConnection", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "2");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        id.properties.setProperty("Adapter4.ThreadPerConnection", "1");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter4", "default");
        ident = comm.stringToIdentity("adapter4");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, true), ident));
        adapter.activate();
        _adapters[3] = proxy;
    }

    public Test.AdapterPrx[]
    getAdapters(Ice.Current current)
    {
        return _adapters;
    }

    public void
    ping(Ice.Current current)
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        for(int i = 0; i < _adapters.length; ++i)
        {
            _adapters[i].ice_getCommunicator().destroy();
        }
        test(_hook.activeThreads() == 0);
        current.adapter.getCommunicator().shutdown();
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private Test.AdapterPrx[] _adapters;
    private ThreadHookI _hook;
}
