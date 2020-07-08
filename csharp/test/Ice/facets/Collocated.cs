//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Facets
{
    public class Collocated : TestHelper
    {
        public override Task Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var d = new D();
            adapter.Add("d", d);
            adapter.Add("d", "facetABCD", d);
            var f = new F();
            adapter.Add("d", "facetEF", f);
            var h = new H(communicator);
            adapter.Add("d", "facetGH", h);
            AllTests.allTests(this);
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
