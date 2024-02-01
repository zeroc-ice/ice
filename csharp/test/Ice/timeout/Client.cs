//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace timeout
    {
        public class Client : global::Test.TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var properties = createTestProperties(ref args);

                //
                // For this test, we want to disable retries.
                //
                properties.setProperty("Ice.RetryIntervals", "-1");

                //
                // This test kills connections, so we don't want warnings.
                //
                properties.setProperty("Ice.Warn.Connections", "0");

                //
                // Limit the send buffer size, this test relies on the socket
                // send() blocking after sending a given amount of data.
                //
                properties.setProperty("Ice.TCP.SndSize", "50000");
                using var communicator = initialize(properties);
                await AllTests.allTests(this);
            }

            public static Task<int> Main(string[] args) =>
                global::Test.TestDriver.runTestAsync<Client>(args);
        }
    }
}
