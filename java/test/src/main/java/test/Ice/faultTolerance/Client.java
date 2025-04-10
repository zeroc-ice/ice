// Copyright (c) ZeroC, Inc.

package test.Ice.faultTolerance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    private static void usage() {
        System.err.println("Usage: Client port...");
    }

    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        Properties properties = createTestProperties(args, remainingArgs);
        properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        //
        // This test aborts servers, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        try (Communicator communicator = initialize(properties)) {
            List<Integer> ports = new ArrayList<>(args.length);
            for (String arg : remainingArgs) {
                if (arg.charAt(0) == '-') {
                    usage();
                    throw new IllegalArgumentException("Client: unknown option `" + arg + "'");
                }
                ports.add(Integer.parseInt(arg));
            }

            if (ports.isEmpty()) {
                usage();
                throw new RuntimeException("Client: no ports specified");
            }

            int[] arr = new int[ports.size()];
            for (int i = 0; i < arr.length; i++) {
                arr[i] = ports.get(i).intValue();
            }
            AllTests.allTests(this, arr);
        }
    }
}
