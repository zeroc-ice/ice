//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.hold.Test;

namespace Ice.hold
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(ref args);
            Timer timer = new Timer();

            communicator.SetProperty("TestAdapter1.Endpoints", getTestEndpoint(0));
            communicator.SetProperty("TestAdapter1.ThreadPool.Size", "5");
            communicator.SetProperty("TestAdapter1.ThreadPool.SizeMax", "5");
            communicator.SetProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
            communicator.SetProperty("TestAdapter1.ThreadPool.Serialize", "0");
            ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
            adapter1.Add(new Hold(timer, adapter1), "hold");

            communicator.SetProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.SetProperty("TestAdapter2.ThreadPool.Size", "5");
            communicator.SetProperty("TestAdapter2.ThreadPool.SizeMax", "5");
            communicator.SetProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
            communicator.SetProperty("TestAdapter2.ThreadPool.Serialize", "1");
            ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            adapter2.Add(new Hold(timer, adapter2), "hold");

            adapter1.Activate();
            adapter2.Activate();
            serverReady();
            communicator.waitForShutdown();

            timer.shutdown();
            timer.waitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
