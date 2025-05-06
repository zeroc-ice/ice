// Copyright (c) ZeroC, Inc.

package test.Ice.enums;

import com.zeroc.Ice.Communicator;

import test.Ice.enums.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
