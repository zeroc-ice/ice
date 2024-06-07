// Copyright (c) ZeroC, Inc.

#nullable enable

using Test;

namespace Ice.middleware;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) => TestDriver.runTestAsync<Client>(args);
}
