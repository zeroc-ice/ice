// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object d = new DI();
            adapter.add(d, Util.stringToIdentity("d"));
            adapter.addFacet(d, Util.stringToIdentity("d"), "facetABCD");
            Object f = new FI();
            adapter.addFacet(f, Util.stringToIdentity("d"), "facetEF");
            Object h = new HI(communicator);
            adapter.addFacet(h, Util.stringToIdentity("d"), "facetGH");

            AllTests.allTests(this);
        }
    }
}
