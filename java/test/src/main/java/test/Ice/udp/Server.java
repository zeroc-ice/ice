// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Server extends TestHelper {
    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var properties = createTestProperties(args, remainingArgs);
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        boolean isIpv6 = "1".equals(properties.getIceProperty("Ice.IPv6"));

        try (Communicator communicator = initialize(properties)) {
            int num = remainingArgs.size() == 1 ? Integer.parseInt(remainingArgs.get(0)) : 0;

            communicator
                .getProperties()
                .setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
            ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new TestIntfI(), new Identity("control", ""));
            adapter.activate();

            if (num == 0) {
                properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
                ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
                adapter2.add(new TestIntfI(), new Identity("test", ""));
                adapter2.activate();
            }

            StringBuilder endpoint = new StringBuilder();
            if (isIpv6) {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(getTestPort(10));
            } else {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(getTestPort(10));
            }
            properties.setProperty("McastTestAdapter.Endpoints", endpoint.toString());

            // Not used for IPv6 + Android emulator, but doesn't fail either.
            ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), new Identity("test", ""));
            mcastAdapter.activate();

            serverReady();

            communicator.waitForShutdown();
        }
    }
}
