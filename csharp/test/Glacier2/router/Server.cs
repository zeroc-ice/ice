// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c1/callback")); // The test allows "c1" as category.
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c2/callback")); // The test allows "c2" as category.
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c3/callback")); // The test rejects "c3" as category.
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
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
