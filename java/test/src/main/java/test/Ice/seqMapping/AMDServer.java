//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.seqMapping;

public class AMDServer extends test.TestHelper
{
    @Override
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping.AMD");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMyClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
