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

public class Server
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            communicator().getProperties().setProperty("DeactivatedAdapter.Endpoints", "default -p 12011");
            communicator().createObjectAdapter("DeactivatedAdapter");

            communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "default -p 12010");
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
            adapter.add(new CallbackI(), communicator().stringToIdentity("callback"));
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
