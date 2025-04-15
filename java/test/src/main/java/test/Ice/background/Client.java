// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;

import java.util.Collections;

import test.Ice.background.PluginFactory.PluginI;
import test.Ice.background.Test.BackgroundPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        //
        // For this test, we want to disable retries.
        //
        properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        //
        // This test relies on filling the TCP send/recv buffer, so we rely on a fixed value for
        // these buffers.
        //
        properties.setProperty("Ice.TCP.SndSize", "50000");

        properties.setProperty(
            "Ice.Default.Protocol",
            "test-" + properties.getIceProperty("Ice.Default.Protocol"));

        properties.setProperty("Ice.Package.Test", "test.Ice.background");

        // Install transport plug-in.
        var initData = new InitializationData();
        initData.properties = properties;
        initData.pluginFactories = Collections.singletonList(new PluginFactory());

        try (Communicator communicator = initialize(initData)) {
            PluginI plugin = (PluginI) communicator().getPluginManager().getPlugin("Test");
            BackgroundPrx background = AllTests.allTests(plugin.getConfiguration(), this);
            background.shutdown();
        }
    }
}
