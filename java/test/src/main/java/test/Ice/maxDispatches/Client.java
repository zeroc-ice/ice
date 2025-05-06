// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        try (var communicator = initialize(args)) {
            AllTests.allTests(this);
        }
    }
}
