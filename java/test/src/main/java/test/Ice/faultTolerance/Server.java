// Copyright (c) ZeroC, Inc.

package test.Ice.faultTolerance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Server extends TestHelper {
    private static void usage() {
        System.err.println("Usage: Server port");
    }

    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var properties = createTestProperties(args, remainingArgs);

        try (Communicator communicator = initialize(properties)) {
            int port = 0;
            for (String arg : remainingArgs) {
                if (arg.charAt(0) == '-') {
                    usage();
                    throw new RuntimeException("Server: unknown option `" + arg + "'");
                }

                if (port > 0) {
                    usage();
                    throw new RuntimeException("Server: only one port can be specified");
                }

                port = Integer.parseInt(arg);
            }

            if (port <= 0) {
                usage();
                throw new RuntimeException("Server: no port specified");
            }

            // Don't move this, it needs the port.
            communicator
                .getProperties()
                .setProperty("TestAdapter.Endpoints", getTestEndpoint(port));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object object = new TestI(port);
            adapter.add(object, Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
