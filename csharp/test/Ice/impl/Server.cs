//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace ZeroC.Ice.Test.Impl
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using (var communicator = initialize(ref args))
            {
                //
                // We don't want connection warnings because of the timeout test.
                //
                communicator.getProperties().setProperty("Ice.Warn.Connections", "0");

                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.add(Ice.Util.stringToIdentity("test"), new MyDerivedClassI());
                adapter.activate();

                communicator.waitForShutdown();
            }
        }

        public static int Main(string[] args)
        {
            return TestDriver.runTest<Server>(args);
        }
    }
}
