// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.networkProxy;

import test.Ice.networkProxy.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.networkProxy");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
    }
}
