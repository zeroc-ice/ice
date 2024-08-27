// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

public class Client extends test.TestHelper {
  @Override
  public void run(String[] args) {
    var properties = createTestProperties(args);
    properties.setProperty("Ice.Package.Test", "test.Ice.maxConnections");

    try (var communicator = initialize(properties)) {
      AllTests.allTests(this);
    }
  }
}
