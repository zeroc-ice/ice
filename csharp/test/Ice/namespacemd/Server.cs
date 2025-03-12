// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.namespacemd
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using (var communicator = initialize(ref args))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                var adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.add(new InitialI(), Ice.Util.stringToIdentity("initial"));
                adapter.activate();
                serverReady();
                communicator.waitForShutdown();
            }
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Server>(args);
    }
}
