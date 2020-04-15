//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.namespacemd.Test;

namespace Ice
{
    namespace namespacemd
    {
        public class Server : TestHelper
        {
            public override void Run(string[] args)
            {
                using var communicator = Initialize(ref args);
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
                var adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.Add("initial", new Initial());
                adapter.Activate();
                ServerReady();
                communicator.WaitForShutdown();
            }

            public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
        }
    }
}
