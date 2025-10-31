// Copyright (c) ZeroC, Inc.

package test.Ice.info;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", getTestEndpoint(0) + ":" + getTestEndpoint(0, "udp"));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), new Identity("test", ""));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
