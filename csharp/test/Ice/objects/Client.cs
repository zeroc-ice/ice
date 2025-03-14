// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.objects;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        using var communicator = initialize(initData);
        var initial = Test.AllTests.allTests(this);
        initial.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
