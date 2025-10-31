// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.slicing.exceptions;

public class Client : Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using Communicator communicator = initialize(ref args);
        TestIntfPrx test = await AllTests.allTests(this, false);
        test.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
