//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using Test;

namespace Ice.invoke
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            bool async = args.Any(v => v.Equals("--async"));
            using var communicator = initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            if (async)
            {
                adapter.AddDefault(new BlobjectAsyncI());
            }
            else
            {
                adapter.AddDefault(new BlobjectI());
            }
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
