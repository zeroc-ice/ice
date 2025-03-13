// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.facets;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object d = new DI();
        adapter.add(d, Ice.Util.stringToIdentity("d"));
        adapter.addFacet(d, Ice.Util.stringToIdentity("d"), "facetABCD");
        Ice.Object f = new FI();
        adapter.addFacet(f, Ice.Util.stringToIdentity("d"), "facetEF");
        Ice.Object h = new HI(communicator);
        adapter.addFacet(h, Ice.Util.stringToIdentity("d"), "facetGH");
        await AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
