// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import test.Ice.exceptions.Test.ThrowerPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        try(Ice.Communicator communicator = initialize(properties))
        {
            ThrowerPrx thrower = AllTests.allTests(this);
            thrower.shutdown();
        }
    }
}
