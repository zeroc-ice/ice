// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.enums;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        Test.TestIntfPrx proxy = AllTests.allTests(this);
        proxy.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
