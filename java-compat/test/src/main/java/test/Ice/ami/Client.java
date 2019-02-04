//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.ami;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        properties.setProperty("Ice.Warn.AMICallback", "0");
        properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.SndSize", "50000");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this, false);
        }
    }
}
