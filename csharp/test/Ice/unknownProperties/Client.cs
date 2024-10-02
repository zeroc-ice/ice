// Copyright (c) ZeroC, Inc.

namespace Ice.unknownProperties;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        using Communicator communicator = initialize(initData);
        AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
