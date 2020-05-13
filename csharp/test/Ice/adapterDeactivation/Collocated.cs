//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class Collocated : TestHelper
        {
            public override void Run(string[] args)
            {
                using Communicator communicator = Initialize(ref args);
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.AddDefault(new Servant());

                AllTests.allTests(this);

                adapter.WaitForDeactivate();
            }

            public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
        }
    }
}
