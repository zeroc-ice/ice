// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Communicator;

import test.Ice.inheritance.Test.InitialPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
