// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        properties.setProperty("Ice.BatchAutoFlushSize", "100");
        try(Ice.Communicator communicator = initialize(properties))
        {
            java.io.PrintWriter out = getWriter();
            MyClassPrx myClass = AllTests.allTests(this);

            out.print("testing server shutdown... ");
            out.flush();
            myClass.shutdown();
            try
            {
                myClass.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
                throw new RuntimeException();
            }
            catch(Ice.LocalException ex)
            {
                out.println("ok");
            }
        }
    }
}
