// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.servantLocator
{
    public class Collocated : TestHelper
    {
        public override void run(string[] args)
        {
            var initData = new InitializationData();
            initData.properties = createTestProperties(ref args);
            using (var communicator = initialize(initData))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0");

                var adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.addServantLocator(new ServantLocatorI("category"), "category");
                adapter.addServantLocator(new ServantLocatorI(""), "");
                adapter.add(new TestI(), Ice.Util.stringToIdentity("asm"));
                adapter.add(new TestActivationI(), Ice.Util.stringToIdentity("test/activation"));
                AllTests.allTests(this);
            }
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Collocated>(args);
    }
}
