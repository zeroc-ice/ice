// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.exceptions;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        using var communicator = initialize(initData);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var thrower = await AllTests.allTests(this);
        thrower.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
