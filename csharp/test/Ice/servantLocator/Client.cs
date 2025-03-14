// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.servantLocator;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        using var communicator = initialize(initData);
        var obj = AllTests.allTests(this);
        obj.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
