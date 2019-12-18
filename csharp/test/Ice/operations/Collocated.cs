//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.operations.Test;

namespace Ice
{
    namespace operations
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties["Ice.ThreadPool.Client.Size"] = "2";
                properties["Ice.ThreadPool.Client.SizeWarn"] = "0";

                using var communicator = initialize(properties, typeIdNamespaces: new string[] { "Ice.operations.TypeId" });
                communicator.SetProperty("TestAdapter.AdapterId", "test");
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                var prx = adapter.Add(new MyDerivedClassI(), "test");
                //adapter.activate(); // Don't activate OA to ensure collocation is used.

                if (prx.GetConnection() != null)
                {
                    throw new System.Exception();
                }
                AllTests.allTests(this);
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
