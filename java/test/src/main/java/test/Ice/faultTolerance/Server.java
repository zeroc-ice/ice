// Copyright (c) ZeroC, Inc.

package test.Ice.faultTolerance;

public class Server extends test.TestHelper {
    private static void usage() {
        System.err.println("Usage: Server port");
    }

    public void run(String[] args) {
        java.util.List<String> remainingArgs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, remainingArgs);
        properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
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
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object object = new TestI(port);
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
