// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

public class AMDServer extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.objects.serverAMD");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator
                    .getProperties()
                    .setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 2000");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDTestI(), com.zeroc.Ice.Util.stringToIdentity("Test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
