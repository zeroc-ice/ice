// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator,
        Ice.InitializationData initData)
    {
        //
        // Register the server manager. The server manager creates a new
        // 'server' (a server isn't a different process, it's just a new
        // communicator and object adapter).
        //
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        properties.setProperty("ServerManagerAdapter.Endpoints", "default -p 12010:udp");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

        //
        // We also register a sample server locator which implements the
        // locator interface, this locator is used by the clients and the
        // 'servers' created with the server manager interface.
        //
        ServerLocatorRegistry registry = new ServerLocatorRegistry();
        Ice.Object @object = new ServerManagerI(registry, initData);
        adapter.add(@object, Ice.Util.stringToIdentity("ServerManager"));
        registry.addObject(adapter.createProxy(Ice.Util.stringToIdentity("ServerManager")));
        Ice.LocatorRegistryPrx registryPrx = Ice.LocatorRegistryPrxHelper.uncheckedCast(
                                                adapter.add(registry, Ice.Util.stringToIdentity("registry")));

        ServerLocator locator = new ServerLocator(registry, registryPrx);
        adapter.add(locator, Ice.Util.stringToIdentity("locator"));

        adapter.activate();
        communicator.waitForShutdown();

        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator, initData);
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
