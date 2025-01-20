// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(adapter), com.zeroc.Ice.Util.stringToIdentity("initial"));
            adapter.add(new F2I(), com.zeroc.Ice.Util.stringToIdentity("F21"));
            adapter.add(
                    new UnexpectedObjectExceptionTestI(),
                    com.zeroc.Ice.Util.stringToIdentity("uoet"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
