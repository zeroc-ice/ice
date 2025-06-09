// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;

import test.Ice.background.PluginFactory.PluginI;
import test.Ice.background.Test.BackgroundPrx;
import test.TestHelper;

import java.util.Collections;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        // Install transport plug-in.
        initData.pluginFactories = Collections.singletonList(new PluginFactory());

        initData.properties = createTestProperties(args);
        // For this test, we want to disable retries.
        initData.properties.setProperty("Ice.RetryIntervals", "-1");
        // This test kills connections, so we don't want warnings.
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        // This test relies on filling the TCP send/recv buffer, so we rely on a fixed value for these buffers.
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");
        initData.properties.setProperty(
            "Ice.Default.Protocol",
            "test-" + initData.properties.getIceProperty("Ice.Default.Protocol"));

        try (Communicator communicator = initialize(initData)) {
            PluginI plugin = (PluginI) communicator().getPluginManager().getPlugin("Test");
            BackgroundPrx background = AllTests.allTests(plugin.getConfiguration(), this);
            background.shutdown();
        }
    }
}
