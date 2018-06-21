// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.objects;

import test.Ice.slicing.objects.client.Test.TestIntfPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.objects.client");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
