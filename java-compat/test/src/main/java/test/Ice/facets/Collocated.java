// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object d = new DI();
            adapter.add(d, Ice.Util.stringToIdentity("d"));
            adapter.addFacet(d, Ice.Util.stringToIdentity("d"), "facetABCD");
            Ice.Object f = new FI();
            adapter.addFacet(f, Ice.Util.stringToIdentity("d"), "facetEF");
            Ice.Object h = new HI(communicator);
            adapter.addFacet(h, Ice.Util.stringToIdentity("d"), "facetGH");

            AllTests.allTests(this);
        }
    }
}
