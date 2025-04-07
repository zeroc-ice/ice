// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.udp.Test.*;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> rargs = new ArrayList<String>();
        Properties properties = createTestProperties(args, rargs);
        properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
            int num;
            try {
                num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 1;
            } catch (NumberFormatException ex) {
                num = 1;
            }

            for (int i = 0; i < num; i++) {
                var prx =
                        TestIntfPrx.createProxy(
                                communicator(), "control:" + getTestEndpoint(i, "tcp"));
                prx.shutdown();
            }
        }
    }
}
