// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.operations;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        await using Communicator communicator = initialize(initData);
        Test.MyClassPrx myClass = await AllTests.allTests(this);

        Console.Out.Write("testing server shutdown... ");
        Console.Out.Flush();
        myClass.shutdown();
        try
        {
            myClass.ice_invocationTimeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            test(false);
        }
        catch (Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
