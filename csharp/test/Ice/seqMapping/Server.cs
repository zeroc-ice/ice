// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.seqMapping
{
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new MyClassI(), Ice.Util.stringToIdentity("test"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }

