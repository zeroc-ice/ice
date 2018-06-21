// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

import test.Ice.checksum.Test.ChecksumPrx;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.checksum");
        try(Ice.Communicator communicator = initialize(properties))
        {
            ChecksumPrx checksum = AllTests.allTests(this, false);
            checksum.shutdown();
        }
    }
}
