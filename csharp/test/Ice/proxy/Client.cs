// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.proxy;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using Communicator communicator = initialize(ref args);
        Test.MyClassPrx myClass = await AllTests.allTests(this);
        myClass.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
