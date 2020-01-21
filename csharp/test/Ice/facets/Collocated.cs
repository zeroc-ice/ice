//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.facets.Test;

namespace Ice.facets
{
    public class Collocated : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var d = new D();
            adapter.Add(d, "d");
            adapter.Add(d, "d", "facetABCD");
            var f = new F();
            adapter.Add(f, "d", "facetEF");
            var h = new H(communicator);
            adapter.Add(h, "d", "facetGH");
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Collocated>(args);
    }
}
