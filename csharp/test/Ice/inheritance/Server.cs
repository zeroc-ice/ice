//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.inheritance.Test;

namespace Ice
{
    namespace inheritance
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    var initial = new InitialI(adapter);
                    adapter.Add(initial, "initial");
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
