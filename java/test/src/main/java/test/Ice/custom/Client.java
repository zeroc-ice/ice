// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.custom;

import test.Ice.custom.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.custom");
        properties.setProperty("Ice.CacheMessageBuffers", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
