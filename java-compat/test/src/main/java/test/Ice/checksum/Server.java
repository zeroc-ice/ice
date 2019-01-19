//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.checksum;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.checksum.server");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object object = new ChecksumI();
            adapter.add(object, Ice.Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
