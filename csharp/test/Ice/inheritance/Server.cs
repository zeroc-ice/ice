//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Inheritance
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var initial = new InitialI(adapter);
            adapter.Add("initial", initial);
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
