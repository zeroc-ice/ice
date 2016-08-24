// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

public class Server extends test.Util.Application
{
    private Ice.InitializationData _initData;

    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        
        //
        // Register the server manager. The server manager creates a new
        // 'server' (a server isn't a different process, it's just a new
        // communicator and object adapter).
        //
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

        //
        // We also register a sample server locator which implements the
        // locator interface, this locator is used by the clients and the
        // 'servers' created with the server manager interface.
        //
        ServerLocatorRegistry registry = new ServerLocatorRegistry();
        registry.addObject(adapter.createProxy(Ice.Util.stringToIdentity("ServerManager")));
        Ice.Object object = new ServerManagerI(registry, _initData, this);
        adapter.add(object, Ice.Util.stringToIdentity("ServerManager"));

        Ice.LocatorRegistryPrx registryPrx = Ice.LocatorRegistryPrxHelper.uncheckedCast(adapter.add(registry,
                Ice.Util.stringToIdentity("registry")));

        ServerLocator locator = new ServerLocator(registry, registryPrx);
        adapter.add(locator, Ice.Util.stringToIdentity("locator"));

        adapter.activate();

        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.location");
        initData.properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        initData.properties.setProperty("ServerManagerAdapter.Endpoints", "default -p 12010:udp");

        _initData = initData;
        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
