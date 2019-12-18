//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.servantLocator.Test;

namespace Ice
{
    namespace servantLocator
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.servantLocator.TypeId" });
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.SetProperty("Ice.Warn.Dispatch", "0");

                var adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.AddServantLocator(new ServantLocatorI("category"), "category");
                adapter.AddServantLocator(new ServantLocatorI(""), "");
                adapter.Add(new TestI(), "asm");
                adapter.Add(new TestActivationI(), "test/activation");
                AllTests.allTests(this);
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
