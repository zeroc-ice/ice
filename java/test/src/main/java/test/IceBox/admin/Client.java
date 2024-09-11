// Copyright (c) ZeroC, Inc.

package test.IceBox.admin;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Default.Host", "127.0.0.1");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);

            // Shutdown the IceBox server.
            var prx =
                com.zeroc.Ice.ProcessPrx.createProxy(
                    communicator(), "DemoIceBox/admin -f Process:default -p 9996");
            prx.shutdown();
        }
    }
}
