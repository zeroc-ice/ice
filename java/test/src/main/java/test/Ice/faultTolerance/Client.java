// Copyright (c) ZeroC, Inc.

package test.Ice.faultTolerance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    private static void usage() {
        System.err.println("Usage: Client port...");
    }

    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.faultTolerance.Test");
        initData.properties = createTestProperties(args, remainingArgs);
        // This test aborts servers, so we don't want warnings.
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        try (Communicator communicator = initialize(initData)) {
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
