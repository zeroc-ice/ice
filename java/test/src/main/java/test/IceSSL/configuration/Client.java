// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;

import test.IceSSL.configuration.Test.ServerFactoryPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        if(rargs.size() < 1)
        {
            throw new RuntimeException("Usage: client testdir");
        }
        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            ServerFactoryPrx factory = AllTests.allTests(this, rargs.get(0));
            factory.shutdown();
        }
    }
}
