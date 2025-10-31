// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object d = new DI();
            adapter.add(d, new Identity("d", ""));
            adapter.addFacet(d, new Identity("d", ""), "facetABCD");
            Object f = new FI();
            adapter.addFacet(f, new Identity("d", ""), "facetEF");
            Object h = new HI(communicator);
            adapter.addFacet(h, new Identity("d", ""), "facetGH");

            AllTests.allTests(this);
        }
    }
}
