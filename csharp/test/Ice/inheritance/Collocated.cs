// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.inheritance;

public class Collocated : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new InitialI(adapter), Ice.Util.stringToIdentity("initial"));
        AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
