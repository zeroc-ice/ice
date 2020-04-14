//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

namespace Ice.exceptions
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            string[] typeIdNamespaces = new string[] { "Ice.exceptions.TypeId" };
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.MessageSizeMax"] = "10"; // 10KB max
            using Communicator communicator = Initialize(properties, typeIdNamespaces: typeIdNamespaces);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            Test.IThrowerPrx thrower = AllTests.allTests(this);
            thrower.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
