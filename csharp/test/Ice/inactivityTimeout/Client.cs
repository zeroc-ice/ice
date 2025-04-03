// Copyright (c) ZeroC, Inc.

namespace Ice.inactivityTimeout;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var properties = createTestProperties(ref args);

        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the sending of the
        // heartbeats that schedules the inactivity timer task.
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
        properties.setProperty("Ice.Connection.Client.InactivityTimeout", "3");
        await using var communicator = initialize(properties);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
