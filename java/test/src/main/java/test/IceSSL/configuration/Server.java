// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Server extends TestHelper {
    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var properties = createTestProperties(args, remainingArgs);
        if (remainingArgs.size() < 1) {
            throw new RuntimeException("Usage: server testdir");
        }

        try (Communicator communicator = initialize(properties)) {
            communicator
                .getProperties()
                .setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "tcp"));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Identity id = Util.stringToIdentity("factory");
            adapter.add(new ServerFactoryI(remainingArgs.get(0)), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
