// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxConnections;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var properties = createTestProperties(ref args);

        // Reduce connect timeout to fail quickly if the server is not accepting connections.
        properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
        using var communicator = initialize(properties);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
