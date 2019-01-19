//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.client.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.exceptions.client");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
