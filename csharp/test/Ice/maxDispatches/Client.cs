// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using var communicator = initialize(ref args);
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
