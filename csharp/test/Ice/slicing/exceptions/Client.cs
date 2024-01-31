//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Threading.Tasks;

public class Client : Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        using var communicator = initialize(ref args);
        TestIntfPrx test = await AllTests.allTests(this, false);
        test.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
