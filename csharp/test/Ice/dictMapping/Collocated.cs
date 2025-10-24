// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.dictMapping;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using Communicator communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyClassI(), Util.stringToIdentity("test"));
        // Don't activate OA to ensure collocation is used.
        await AllTests.allTests(this, true);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
