// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        try(Ice.Communicator communicator = initialize(properties))
        {
            java.io.PrintWriter out = getWriter();
            MyClassPrx myClass = AllTests.allTests(this, false);

            out.print("shutting down server... ");
            out.flush();
            myClass.shutdown();
            out.println("ok");
        }
    }
}
