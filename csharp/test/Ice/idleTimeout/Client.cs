// Copyright (c) ZeroC, Inc.

namespace Ice.idleTimeout;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties.setProperty("Ice.Connection.IdleTimeout", "1");

        // TODO: temporary work-around for IceSSL always sending messages asynchronously.
        properties.setProperty("Ice.Connection.EnableIdleCheck", "0");
        using var communicator = initialize(properties);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
