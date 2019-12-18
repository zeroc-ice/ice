//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.facets.Test;

namespace Ice
{
    namespace facets
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    var d = new DI();
                    adapter.Add(d, "d");
                    adapter.Add(d, "d", "facetABCD");
                    var f = new FI();
                    adapter.Add(f, "d", "facetEF");
                    var h = new HI(communicator);
                    adapter.Add(h, "d", "facetGH");
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
