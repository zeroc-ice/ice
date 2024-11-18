// Copyright (c) ZeroC, Inc.

namespace Ice.timeout
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

            properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
            properties.setProperty("Ice.Connection.Client.CloseTimeout", "1");

            using var communicator = initialize(properties);
            await AllTests.allTests(this);
        }

        public static Task<int> Main(string[] args) =>
            global::Test.TestDriver.runTestAsync<Client>(args);
    }
}
