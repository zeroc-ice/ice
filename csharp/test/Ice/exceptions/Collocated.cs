// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.exceptions;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        // No need to set connection properties such as Ice.Warn.Connections or Ice.MessageSizeMax.
        await using Communicator communicator = initialize(initData);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object obj = new ThrowerI();
        adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
