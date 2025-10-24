// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.namespacemd;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        using Communicator communicator = initialize(properties);
        Test.InitialPrx initial = AllTests.allTests(this);
        initial.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
