// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceMulticastServer")]
[assembly: AssemblyDescription("Ice multicast demo server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            args = communicator().getProperties().parseCommandLineOptions("Discover", args);

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
            Ice.ObjectAdapter discoverAdapter = communicator().createObjectAdapter("Discover");

            Ice.ObjectPrx hello = adapter.addWithUUID(new HelloI());
            discoverAdapter.add(new DiscoverI(hello), communicator().stringToIdentity("discover"));

            discoverAdapter.activate();        
            adapter.activate();
            
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.server");
    }
}
