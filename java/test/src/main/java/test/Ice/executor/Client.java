// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.executor");

        CustomExecutor executor = new CustomExecutor();
        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        initData.executor = executor;
        try (com.zeroc.Ice.Communicator communicator = initialize(initData)) {
            AllTests.allTests(this, executor);
        }
        executor.terminate();
    }
}
