//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace facets
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Object d = new DI();
                    adapter.add(d, Ice.Util.stringToIdentity("d"));
                    adapter.addFacet(d, Ice.Util.stringToIdentity("d"), "facetABCD");
                    Ice.Object f = new FI();
                    adapter.addFacet(f, Ice.Util.stringToIdentity("d"), "facetEF");
                    Ice.Object h = new HI(communicator);
                    adapter.addFacet(h, Ice.Util.stringToIdentity("d"), "facetGH");
                    adapter.activate();
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
