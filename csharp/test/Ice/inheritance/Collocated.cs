//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.inheritance.Test;

namespace Ice.inheritance
{
    public class Collocated : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("initial", new InitialI(adapter));
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Collocated>(args);
    }
}
