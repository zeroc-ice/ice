// Copyright (c) ZeroC, Inc.

package test.Ice.wsAllowedOrigins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            // The Java test framework's TestHelper only consumes --Test.* args; this test relies on the framework
            // passing --TestAdapter.AllowedOrigins=..., so we consume the TestAdapter prefix explicitly here.
            communicator.getProperties().parseCommandLineOptions("TestAdapter", args);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "ws"));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
