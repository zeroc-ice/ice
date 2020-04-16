//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.dictMapping.AMD
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyClass());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
