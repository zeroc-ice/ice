// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.inheritance;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        Test.InitialPrx initial = AllTests.allTests(this);
        initial.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
