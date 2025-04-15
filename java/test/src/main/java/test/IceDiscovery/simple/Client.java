// Copyright (c) ZeroC, Inc.

package test.IceDiscovery.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> rargs = new ArrayList<String>();
        Properties properties = createTestProperties(args, rargs);
        try (Communicator communicator = initialize(properties)) {
            int num;
            try {
                num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 0;
            } catch (NumberFormatException ex) {
                num = 0;
            }
            AllTests.allTests(this, num);
        }
    }
}
