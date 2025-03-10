// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.ami
{


        public class Client : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.AMICallback", "0");
                properties.setProperty("Ice.Warn.Connections", "0");

                //
                // We use a client thread pool with more than one thread to test
                // that task inlining works.
                //
                properties.setProperty("Ice.ThreadPool.Client.Size", "5");

                //
                // Limit the send buffer size, this test relies on the socket
                // send() blocking after sending a given amount of data.
                //
                properties.setProperty("Ice.TCP.SndSize", "50000");
                using (var communicator = initialize(properties))
                {
                    await AllTests.allTestsAsync(this, false);
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

