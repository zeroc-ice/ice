//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.exceptions;

public class Collocated extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        initData.logger = new DummyLogger();

        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        try(Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new ThrowerI(), Ice.Util.stringToIdentity("thrower"));
            AllTests.allTests(this);
        }
    }
}
