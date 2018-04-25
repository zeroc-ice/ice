// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        int num = 0;
        try
        {
            num =  Int32.Parse(args[0]);
        }
        catch(FormatException)
        {
        }

        properties.setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
        properties.setProperty("ControlAdapter.AdapterId", "control" + num);
        properties.setProperty("ControlAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControlAdapter");
        adapter.add(new ControllerI(), Ice.Util.stringToIdentity("controller" + num));
        adapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
