//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.MessageSizeMax"] = "10"; // 10KB max
            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("thrower", new Thrower());
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
