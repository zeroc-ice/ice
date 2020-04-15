//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

namespace Ice.exceptions
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            string[] typeIdNamespaces = new string[] { "Ice.exceptions.TypeId" };
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.MessageSizeMax"] = "10"; // 10KB max
            using Communicator communicator = Initialize(properties, typeIdNamespaces: typeIdNamespaces);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("thrower", new Thrower());
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
