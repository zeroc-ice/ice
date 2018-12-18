// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.checksum;

import test.Ice.checksum.Test.ChecksumPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.checksum");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            ChecksumPrx checksum = AllTests.allTests(this, false);
            checksum.shutdown();
        }
    }
}
