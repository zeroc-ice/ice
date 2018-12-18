// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.client.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.exceptions.client");
        try(Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
