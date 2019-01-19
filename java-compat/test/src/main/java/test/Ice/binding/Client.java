//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.binding;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.binding");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
    }
}
