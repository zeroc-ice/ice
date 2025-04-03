// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxConnections;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        // We disable retries to make the logs clearer and avoid hiding potential issues.
        properties.setProperty("Ice.RetryIntervals", "-1");

        await using var communicator = initialize(properties);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
