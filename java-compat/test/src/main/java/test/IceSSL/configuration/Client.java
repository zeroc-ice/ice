// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceSSL.configuration;

import java.io.PrintWriter;

import test.IceSSL.configuration.Test.ServerFactoryPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        PrintWriter out = getWriter();
        if(argsH.value.length < 1)
        {
            throw new RuntimeException("Usage: client testdir");
        }

        try(Ice.Communicator communicator = initialize(properties))
        {
            ServerFactoryPrx factory = AllTests.allTests(this, argsH.value[0]);
            factory.shutdown();
        }
    }
}
