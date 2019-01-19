//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.info;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.info");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
    }
}
