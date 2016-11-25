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

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
        communicator().createObjectAdapter("DeactivatedAdapter");

        communicator().getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("callback"));
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
