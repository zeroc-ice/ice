// Copyright (c) ZeroC, Inc.

using Test;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        TestIntfPrx test = AllTests.allTests(this, false);
        test.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
