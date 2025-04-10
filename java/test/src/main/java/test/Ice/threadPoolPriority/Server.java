// Copyright (c) ZeroC, Inc.

package test.Ice.threadPoolPriority;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.ThreadPool.Server.ThreadPriority", "10");
        try (Communicator communicator = initialize(properties)) {
            communicator
                .getProperties()
                .setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 10000");
            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.add(new PriorityI(), Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
