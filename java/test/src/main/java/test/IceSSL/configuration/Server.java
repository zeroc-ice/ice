// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Server extends TestHelper {
    public void run(String[] args) {
        List<String> rargs = new ArrayList<String>();
        Properties properties = createTestProperties(args, rargs);
        if (rargs.size() < 1) {
            throw new RuntimeException("Usage: server testdir");
        }

        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try (Communicator communicator = initialize(properties)) {
            communicator
                    .getProperties()
                    .setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "tcp"));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Identity id = Util.stringToIdentity("factory");
            adapter.add(new ServerFactoryI(rargs.get(0) + "/../certs"), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
