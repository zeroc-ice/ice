// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        java.io.PrintWriter out = getWriter();
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this, true);
        }
    }
}
