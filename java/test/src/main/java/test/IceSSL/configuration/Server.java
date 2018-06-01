// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        if(rargs.size() < 1)
        {
            throw new RuntimeException("Usage: server testdir");
        }

        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "tcp"));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("factory");
            adapter.add(new ServerFactoryI(rargs.get(0) + "/../certs"), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
