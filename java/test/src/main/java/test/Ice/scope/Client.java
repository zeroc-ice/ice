// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.scope;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        java.io.PrintWriter out = getWriter();
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            out.print("test same Slice type name in different scopes... ");
            out.flush();
            AllTests.allTests(this);
            out.println("ok");
        }
    }
}
