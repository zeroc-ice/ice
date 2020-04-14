//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.admin.Test;

namespace Ice
{
    namespace admin
    {
        public class Server : TestHelper
        {
            public override void Run(string[] args)
            {
                using Communicator communicator = Initialize(ref args);
                communicator.SetProperty("TestAdapter.Endpoints", $"{GetTestEndpoint(0)} -t 10000");
                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.Add("factory", new RemoteCommunicatorFactoryI());
                adapter.Activate();
                ServerReady();
                communicator.WaitForShutdown();
            }

            public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
        }
    }
}
