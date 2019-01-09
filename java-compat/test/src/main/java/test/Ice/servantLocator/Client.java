// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.servantLocator");
        try(Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx obj = AllTests.allTests(this);
            obj.shutdown();
        }
    }
}
