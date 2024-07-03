//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.timeout;

public class Client extends test.TestHelper {
  @Override
  public void run(String[] args) {
    com.zeroc.Ice.Properties properties = createTestProperties(args);
    properties.setProperty("Ice.Package.Test", "test.Ice.timeout");

    //
    // For this test, we want to disable retries.
    //
    properties.setProperty("Ice.RetryIntervals", "-1");

    properties.setProperty("Ice.Connection.ConnectTimeout", "1");
    properties.setProperty("Ice.Connection.CloseTimeout", "1");

    //
    // This test kills connections, so we don't want warnings.
    //
    properties.setProperty("Ice.Warn.Connections", "0");

    try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
      AllTests.allTests(this);
    }
  }
}
