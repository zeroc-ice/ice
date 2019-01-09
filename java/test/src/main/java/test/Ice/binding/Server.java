// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.binding;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.binding");
        initData.logger = new com.zeroc.Ice.Logger() {
            @Override public void print(String message)
            {
            }

            @Override public void trace(String category, String message)
            {
            }

            @Override public void warning(String message)
            {
            }

            @Override public void error(String message)
            {
            }

            @Override public String getPrefix()
            {
                return "NullLogger";
            }

            @Override public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
            {
                return this;
            }
        };

        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("communicator");
            adapter.add(new RemoteCommunicatorI(this), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
