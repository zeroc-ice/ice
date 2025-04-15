// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.util.stream.Stream;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        try (Communicator communicator = initialize(properties)) {
            boolean withDeploy = Stream.of(args).anyMatch(v -> "--with-deploy".equals(v));

            if (!withDeploy) {
                AllTests.allTests(this);
            } else {
                AllTests.allTestsWithDeploy(this);
            }
        }
    }
}
