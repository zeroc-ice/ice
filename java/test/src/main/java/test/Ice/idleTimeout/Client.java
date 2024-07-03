// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

public class Client extends test.TestHelper {
  @Override
  public void run(String[] args) {
    var properties = createTestProperties(args);
    properties.setProperty("Ice.Package.Test", "test.Ice.idleTimeout");
    properties.setProperty("Ice.Connection.IdleTimeout", "1");

    try (var communicator = initialize(properties)) {
      AllTests.allTests(this);
    }
  }
}
