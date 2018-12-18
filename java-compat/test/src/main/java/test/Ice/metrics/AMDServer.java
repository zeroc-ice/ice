// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

public class AMDServer extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Admin.Endpoints", "tcp");
        properties.setProperty("Ice.Admin.InstanceName", "server");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMetricsI(), Ice.Util.stringToIdentity("metrics"));
            adapter.activate();

            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
