//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.facets
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
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
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
