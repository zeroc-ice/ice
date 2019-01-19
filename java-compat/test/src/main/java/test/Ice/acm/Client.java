//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.acm;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.acm");
        properties.setProperty("Ice.Warn.Connections", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
    }
}
