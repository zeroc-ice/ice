// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.timeout");

        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                    .getProperties()
                    .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TimeoutI(), com.zeroc.Ice.Util.stringToIdentity("timeout"));
            adapter.activate();

            com.zeroc.Ice.ObjectAdapter controllerAdapter =
                    communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                    new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
