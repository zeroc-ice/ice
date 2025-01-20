// Copyright (c) ZeroC, Inc.

package test.Ice.defaultServant;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        try (com.zeroc.Ice.Communicator communicator = initialize(args)) {
            AllTests.allTests(this);
        }
    }
}
