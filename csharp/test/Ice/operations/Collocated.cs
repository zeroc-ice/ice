//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.operations.Test;

namespace Ice.operations
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);

            using var communicator = Initialize(properties, typeIdNamespaces: new string[] { "Ice.operations.TypeId" });
            communicator.SetProperty("TestAdapter.AdapterId", "test");
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var prx = adapter.Add("test", new MyDerivedClass(), IMyDerivedClassPrx.Factory);
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            if (prx.GetConnection() != null)
            {
                Assert(false);
                throw new System.Exception();
            }
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
