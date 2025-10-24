// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.facets;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using Communicator communicator = initialize(ref args);
        Test.GPrx g = await AllTests.allTests(this);
        g.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
