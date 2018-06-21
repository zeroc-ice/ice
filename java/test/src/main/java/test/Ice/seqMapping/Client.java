// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;

public class Client extends test.TestHelper
{
    @Override
    public void run(String[] args)
    {
        java.io.PrintWriter out = getWriter();

        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            MyClassPrx myClass = AllTests.allTests(this, false);

            out.print("shutting down server... ");
            out.flush();
            myClass.shutdown();
            out.println("ok");
        }
    }
}
