// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
    }
}
