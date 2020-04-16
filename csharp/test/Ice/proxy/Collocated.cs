//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.proxy.Test;

namespace Ice.proxy
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.ThreadPool.Client.Size"] = "2"; // For nested AMI.
            properties["Ice.ThreadPool.Client.SizeWarn"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";

            using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyDerivedClass());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
