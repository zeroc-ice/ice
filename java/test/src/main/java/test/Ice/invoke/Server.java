// Copyright (c) ZeroC, Inc.

package test.Ice.invoke;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.util.stream.Stream;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.invoke");
        try (Communicator communicator = initialize(properties)) {
            boolean async = Stream.of(args).anyMatch(v -> "--async".equals(v));
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(async), "");
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
