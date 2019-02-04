//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.enums;

import test.Ice.enums.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.enums");
        try(Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
