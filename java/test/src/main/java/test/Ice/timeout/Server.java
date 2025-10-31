// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TimeoutI(), new Identity("timeout", ""));
            adapter.activate();

            ObjectAdapter controllerAdapter =
                communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                new ControllerI(adapter), new Identity("controller", ""));
            controllerAdapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
