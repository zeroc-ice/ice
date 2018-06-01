// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.location");
        properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(properties, 0));
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
        }
        catch(Exception ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
