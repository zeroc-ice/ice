// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.packagemd;

import test.Ice.packagemd.Test.InitialPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.packagemd");
        properties.setProperty("Ice.Package.Test1", "test.Ice.packagemd");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
