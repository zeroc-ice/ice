//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

namespace Ice
{
    namespace acm
    {
        public class Server : TestHelper
        {
            public override void Run(string[] args)
            {
                Dictionary<string, string> properties = CreateTestProperties(ref args);
                properties["Ice.Warn.Connections"] = "0";
                properties["Ice.ACM.Timeout"] = "1";
                using Communicator communicator = Initialize(properties);
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
                communicator.SetProperty("TestAdapter.ACM.Timeout", "0");
                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.Add("communicator", new RemoteCommunicator());
                adapter.Activate();
                ServerReady();
                communicator.SetProperty("Ice.PrintAdapterReady", "0");
                communicator.WaitForShutdown();
            }

            public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
        }
    }
}
