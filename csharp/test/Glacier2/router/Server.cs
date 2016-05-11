// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
            communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "tcp -p 12010");
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
            adapter.add(new CallbackI(),
                        communicator().stringToIdentity("c1/callback")); // The test allows "c1" as category.
            adapter.add(new CallbackI(),
                        communicator().stringToIdentity("c2/callback")); // The test allows "c2" as category.
            adapter.add(new CallbackI(),
                        communicator().stringToIdentity("c3/callback")); // The test rejects "c3" as category.
            adapter.add(new CallbackI(),
                        communicator().stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(ref args);
        App app = new App();
        return app.main(args, initData);
    }
}
