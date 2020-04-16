//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.inheritance
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("initial", new InitialI(adapter));
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
