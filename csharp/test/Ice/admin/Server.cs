// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.admin
{


        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 10000");
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Identity id = Ice.Util.stringToIdentity("factory");
                    adapter.add(new RemoteCommunicatorFactoryI(), id);
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }

