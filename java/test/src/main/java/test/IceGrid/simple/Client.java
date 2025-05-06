// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Communicator;

import test.TestHelper;

import java.util.stream.Stream;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            boolean withDeploy = Stream.of(args).anyMatch(v -> "--with-deploy".equals(v));

            if (!withDeploy) {
                AllTests.allTests(this);
            } else {
                AllTests.allTestsWithDeploy(this);
            }
        }
    }
}
