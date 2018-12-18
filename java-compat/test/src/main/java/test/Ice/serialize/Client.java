// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.serialize;
import test.Ice.serialize.Test.*;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.serialize");
        try(Ice.Communicator communicator = initialize(properties))
        {
            InitialPrx initial = AllTests.allTests(this, false);
            initial.shutdown();
        }
    }
}
