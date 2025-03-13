// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.proxy;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            var myClass = await AllTests.allTests(this);
            myClass.shutdown();
        }
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
