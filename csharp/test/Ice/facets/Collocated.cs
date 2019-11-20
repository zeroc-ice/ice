//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.facets.Test;

namespace Ice
{
    namespace facets
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    var d = new DI();
                    adapter.Add(d, Util.stringToIdentity("d"));
                    adapter.Add(d, Util.stringToIdentity("d"), "facetABCD");
                    var f = new FI();
                    adapter.Add(f, Util.stringToIdentity("d"), "facetEF");
                    var h = new HI(communicator);
                    adapter.Add(h, Util.stringToIdentity("d"), "facetGH");
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
