// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.operations;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        await using var communicator = initialize(initData);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
        // Don't activate OA to ensure collocation is used.

        if (prx.ice_getConnection() != null)
        {
            throw new System.Exception();
        }

        await AllTests.allTests(this);

        testCollocatedIPv6Invocation(this);
    }

    private static void testCollocatedIPv6Invocation(TestHelper helper)
    {
        TextWriter output = helper.getWriter();
        int port = helper.getTestPort(1);
        output.Write("testing collocated invocation with normalized IPv6 address... ");
        output.Flush();
        using var communicator = Ice.Util.initialize();
        communicator.getProperties().setProperty("TestAdapter.Endpoints", $"tcp -h \"0:0:0:0:0:0:0:1\" -p {port}");
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));

        var prx = Ice.ObjectPrxHelper.createProxy(communicator, $"test:tcp -h \"::1\" -p {port}");
        prx.ice_ping();

        prx = Ice.ObjectPrxHelper.createProxy(communicator, $"test:tcp -h \"0:0:0:0:0:0:0:1\" -p {port}");
        prx.ice_ping();
        output.WriteLine();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
