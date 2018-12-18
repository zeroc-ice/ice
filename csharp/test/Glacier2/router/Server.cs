// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using(var communicator = initialize(ref args))
        {
            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");

            //
            // The test allows "c1" as category.
            //
            adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c1/callback"));

            //
            // The test allows "c2" as category.
            //
            adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c2/callback"));

            //
            // The test rejects "c3" as category.
            //
            adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c3/callback"));

            //
            // The test allows the prefixed userid.
            //
            adapter.add(new CallbackI(), Ice.Util.stringToIdentity("_userid/callback"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
