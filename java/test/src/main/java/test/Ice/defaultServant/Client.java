// Copyright (c) ZeroC, Inc.

package test.Ice.defaultServant;

import com.zeroc.Ice.Communicator;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            AllTests.allTests(this);
        }
    }
}
