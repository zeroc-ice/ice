//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.proxy.AMD.Test;

namespace Ice
{
    namespace proxy
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
                        communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        var adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.Add(new MyDerivedClassI(), "test");
                        adapter.Activate();
                        serverReady();
                        communicator.waitForShutdown();
                    }
                }

                public static int Main(string[] args)
                {
                    return global::Test.TestDriver.runTest<Server>(args);
                }
            }
        }
    }
}
