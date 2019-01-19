//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.info;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.info");
        try(com.zeroc.Ice.Communicator communicator = initialize(args))
        {
            AllTests.allTests(this);
        }
    }
}
