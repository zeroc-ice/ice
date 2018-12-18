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
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object d = new DI();
            adapter.add(d, com.zeroc.Ice.Util.stringToIdentity("d"));
            adapter.addFacet(d, com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
            com.zeroc.Ice.Object f = new FI();
            adapter.addFacet(f, com.zeroc.Ice.Util.stringToIdentity("d"), "facetEF");
            com.zeroc.Ice.Object h = new HI(communicator);
            adapter.addFacet(h, com.zeroc.Ice.Util.stringToIdentity("d"), "facetGH");

            AllTests.allTests(this);
        }
    }
}
