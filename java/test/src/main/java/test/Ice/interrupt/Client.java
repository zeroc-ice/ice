// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.interrupt.Test");
        initData.properties = createTestProperties(args);
        // We need to send messages large enough to cause the transport buffers to fill up.
        initData.properties.setProperty("Ice.MessageSizeMax", "20000");
        // Retry up to 2 times, sleep 2s for the last retry. This is
        // useful to test interrupting the retry sleep
        initData.properties.setProperty("Ice.RetryIntervals", "0 2000");
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);
        } catch (InterruptedException ex) {
            throw new RuntimeException(ex);
        }
    }
}
