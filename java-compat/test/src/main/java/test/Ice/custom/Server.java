// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.custom;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.custom");
        properties.setProperty("Ice.CacheMessageBuffers", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object test = new TestI(communicator);
            adapter.add(test, Ice.Util.stringToIdentity("test"));

            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
