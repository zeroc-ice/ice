//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.objects.Test;

namespace Ice
{
    namespace objects
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties["Ice.Warn.Dispatch"] = "0";
                using var communicator = initialize(properties, typeIdNamespaces: new string[] { "Ice.objects.TypeId" });
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.Add(new InitialI(adapter), "initial");
                adapter.Add(new F2I(), "F21");
                var uoet = new UnexpectedObjectExceptionTestI();
                adapter.Add((incoming, current) => uoet.Dispatch(incoming, current), "uoet");
                Test.AllTests.allTests(this);
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
