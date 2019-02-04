//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceBox.admin;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Default.Host", "127.0.0.1");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(communicator);

            //
            // Shutdown the IceBox server.
            //
            Ice.ObjectPrx prx = communicator.stringToProxy("DemoIceBox/admin -f Process:default -p 9996");
            Ice.ProcessPrxHelper.uncheckedCast(prx).shutdown();
        }
    }
}
