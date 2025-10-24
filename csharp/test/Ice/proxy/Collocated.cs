// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.proxy;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");

        await using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new CI(), Ice.Util.stringToIdentity("c"));
        // Don't activate OA to ensure collocation is used.
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
