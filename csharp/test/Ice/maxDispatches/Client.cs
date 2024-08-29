// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var properties = createTestProperties(ref args);
        using var communicator = initialize(properties);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
