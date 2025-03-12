// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.seqMapping;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        using (var communicator = initialize(initData))
        {
            var myClass = await AllTests.allTests(this, false);
            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            myClass.shutdown();
            Console.Out.WriteLine("ok");
        }
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
