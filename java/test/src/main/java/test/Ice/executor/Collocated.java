// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

public class Collocated extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        CustomExecutor executor = new CustomExecutor();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.executor");
        initData.executor = executor;
        try (com.zeroc.Ice.Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                    .getProperties()
                    .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectAdapter adapter2 =
                    communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(executor), com.zeroc.Ice.Util.stringToIdentity("test"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.
            adapter2.add(
                    new TestControllerI(adapter),
                    com.zeroc.Ice.Util.stringToIdentity("testController"));
            // adapter2.activate(); // Don't activate OA to ensure collocation is used.

            AllTests.allTests(this, executor);
        }
        executor.terminate();
    }
}
