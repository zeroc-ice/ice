// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.GPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            GPrx g = AllTests.allTests(this);
            g.shutdown();
        }
    }
}
