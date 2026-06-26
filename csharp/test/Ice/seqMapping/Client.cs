// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.seqMapping;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        await using Communicator communicator = initialize(initData);
        Test.MyInterfacePrx myInterface = await AllTests.allTests(this, false);
        Console.Out.Write("shutting down server... ");
        Console.Out.Flush();
        myInterface.shutdown();
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
