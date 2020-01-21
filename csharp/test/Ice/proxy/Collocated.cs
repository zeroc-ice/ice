//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.proxy.Test;

namespace Ice.proxy
{
    public class Collocated : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.ThreadPool.Client.Size"] = "2"; // For nested AMI.
            properties["Ice.ThreadPool.Client.SizeWarn"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";

            using var communicator = initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new MyDerivedClass(), "test");
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Collocated>(args);
    }
}
