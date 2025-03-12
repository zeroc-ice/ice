// Copyright (c) ZeroC, Inc.

namespace Ice.proxy
{
    namespace AMD
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                //
                // We don't want connection warnings because of the timeout test.
                //
                properties.setProperty("Ice.Warn.Connections", "0");
                properties.setProperty("Ice.Warn.Dispatch", "0");
                using (var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                global::Test.TestDriver.runTestAsync<Server>(args);
        }
    }
}
