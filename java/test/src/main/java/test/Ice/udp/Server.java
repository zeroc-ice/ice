// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Communicator;
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
        properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        {
            String endpoint;
            if ("1".equals(properties.getIceProperty("Ice.IPv6"))) {
                if (System.getProperty("os.name").contains("OS X")) {
                    endpoint = "udp -h \"ff15::1:1\" -p 12020 --interface \"::1\"";
                } else {
                    endpoint = "udp -h \"ff15::1:1\" -p 12020";
                }
            } else {
                endpoint = "udp -h 239.255.1.1 -p 12020";
            }
            properties.setProperty("McastTestAdapter.Endpoints", endpoint);
        }

        try (Communicator communicator = initialize(properties)) {
            int num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 0;

            communicator
                    .getProperties()
                    .setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
            ObjectAdapter adapter =
                    communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new TestIntfI(), Util.stringToIdentity("control"));
            adapter.activate();

            if (num == 0) {
                properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
                ObjectAdapter adapter2 =
                        communicator.createObjectAdapter("TestAdapter");
                adapter2.add(new TestIntfI(), Util.stringToIdentity("test"));
                adapter2.activate();
            }

            StringBuilder endpoint = new StringBuilder();
            if ("1".equals(properties.getIceProperty("Ice.IPv6"))) {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(getTestPort(10));
                if (System.getProperty("os.name").contains("OS X")
                        || System.getProperty("os.name").startsWith("Windows")) {
                    endpoint.append(
                            " --interface \"::1\""); // Use loopback to prevent other machines to
                    // answer.
                }
            } else {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(getTestPort(10));
                if (System.getProperty("os.name").contains("OS X")
                        || System.getProperty("os.name").startsWith("Windows")) {
                    endpoint.append(
                            " --interface 127.0.0.1"); // Use loopback to prevent other machines to
                    // answer.
                }
            }
            properties.setProperty("McastTestAdapter.Endpoints", endpoint.toString());

            ObjectAdapter mcastAdapter =
                    communicator.createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), Util.stringToIdentity("test"));
            mcastAdapter.activate();

            serverReady();

            communicator.waitForShutdown();
        }
    }
}
