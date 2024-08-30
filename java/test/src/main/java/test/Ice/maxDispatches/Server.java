// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Identity;

public class Server extends test.TestHelper {
  public void run(String[] args) {
    var properties = createTestProperties(args);
    properties.setProperty("Ice.Package.Test", "test.Ice.maxDispatches");
    properties.setProperty(
        "Ice.ThreadPool.Server.Size", "10"); // plenty of threads to handle the requests

    try (var communicator = initialize(properties)) {
      communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
      var adapter = communicator.createObjectAdapter("TestAdapter");
      adapter.add(new TestIntfI(), new Identity("test", ""));
      adapter.activate();

      communicator.getProperties().setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(1));
      communicator.getProperties().setProperty("TestAdapterMax10.Connection.MaxDispatches", "10");
      var adapterMax10 = communicator.createObjectAdapter("TestAdapterMax10");
      adapterMax10.add(new TestIntfI(), new Identity("test", ""));
      adapterMax10.activate();

      communicator.getProperties().setProperty("TestAdapterMax1.Endpoints", getTestEndpoint(2));
      communicator.getProperties().setProperty("TestAdapterMax1.Connection.MaxDispatches", "1");
      var adapterMax1 = communicator.createObjectAdapter("TestAdapterMax1");
      adapterMax1.add(new TestIntfI(), new Identity("test", ""));
      adapterMax1.activate();

      communicator
          .getProperties()
          .setProperty("TestAdapterSerialize.Endpoints", getTestEndpoint(3));
      communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Size", "10");
      communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Serialize", "1");
      var adapterSerialize = communicator.createObjectAdapter("TestAdapterSerialize");
      adapterSerialize.add(new TestIntfI(), new Identity("test", ""));
      adapterSerialize.activate();

      serverReady();
      communicator.waitForShutdown();
    }
  }
}
