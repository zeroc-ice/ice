// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

public class AMDServer extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Admin.Endpoints", "tcp");
        properties.setProperty("Ice.Admin.InstanceName", "server");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");

        try(com.zeroc.Ice.Communicator  communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMetricsI(), com.zeroc.Ice.Util.stringToIdentity("metrics"));
            adapter.activate();

            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            com.zeroc.Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
