// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.udp.Test.TestIntfPrx;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "FAIL_IF_NONE");
        initData.properties = createTestProperties(args, remainingArgs);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties.setProperty("Ice.UDP.SndSize", "16384");

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);
            int num;
            try {
                num = remainingArgs.size() == 1 ? Integer.parseInt(remainingArgs.get(0)) : 1;
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
