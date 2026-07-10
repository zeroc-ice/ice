// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Communicator;

import test.Ice.proxy.Test.MyInterfacePrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            MyInterfacePrx myInterface = AllTests.allTests(this);
            myInterface.shutdown();
        }
    }
}
