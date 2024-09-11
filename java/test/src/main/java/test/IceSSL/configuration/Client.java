// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import test.IceSSL.configuration.Test.ServerFactoryPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        if(rargs.size() < 1)
        {
            throw new RuntimeException("Usage: client testdir");
        }

        String testDir = rargs.get(0);

        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            PlatformTests.allTests(this, testDir);
            ServerFactoryPrx factory = AllTests.allTests(this, testDir);
            factory.shutdown();
        }
    }
}
