// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;

import test.Ice.facets.Test.GPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            GPrx g = AllTests.allTests(this);
            g.shutdown();
        }
    }
}
