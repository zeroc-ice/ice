//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.dictMapping
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyClass());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this, true);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
