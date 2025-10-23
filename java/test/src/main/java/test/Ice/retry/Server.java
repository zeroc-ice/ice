// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");

        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new RetryI(), new Identity("retry", ""));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
