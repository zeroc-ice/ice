// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.invoke;

import java.util.stream.Stream;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.invoke");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            boolean async = Stream.of(args).anyMatch(v -> v.equals("--async"));
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(async), "");
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
