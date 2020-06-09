//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.MessageSizeMax"] = "10"; // 10KB max
            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));
            communicator.SetProperty("TestAdapter2.MessageSizeMax", "0");
            communicator.SetProperty("TestAdapter3.Endpoints", GetTestEndpoint(2));
            communicator.SetProperty("TestAdapter3.MessageSizeMax", "1");

            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter2");
            ObjectAdapter adapter3 = communicator.CreateObjectAdapter("TestAdapter3");
            var obj = new Thrower();
            adapter.Add("thrower", obj);
            adapter2.Add("thrower", obj);
            adapter3.Add("thrower", obj);
            adapter.Activate();
            adapter2.Activate();
            adapter3.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
