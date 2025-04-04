// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import java.util.stream.Stream;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            boolean withDeploy = Stream.of(args).anyMatch(v -> "--with-deploy".equals(v));

            if (!withDeploy) {
                AllTests.allTests(this);
            } else {
                AllTests.allTestsWithDeploy(this);
            }
        }
    }
}
