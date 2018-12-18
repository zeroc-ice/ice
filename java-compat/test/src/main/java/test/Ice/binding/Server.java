
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.binding");
        initData.logger = new Ice.Logger() {
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

            @Override public Ice.Logger cloneWithPrefix(String prefix)
            {
                return this;
            }
        };

        try(Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Identity id = Ice.Util.stringToIdentity("communicator");
            adapter.add(new RemoteCommunicatorI(), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
