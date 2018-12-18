// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.proxy;

import test.Ice.proxy.Test.MyClassPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        try(Ice.Communicator communicator = initialize(properties))
        {
            MyClassPrx myClass = AllTests.allTests(this);
            myClass.shutdown();
        }
    }
}
