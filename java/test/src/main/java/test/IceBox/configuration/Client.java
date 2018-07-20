// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.configuration;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.IceBox.configuration");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);

            //
            // Shutdown the IceBox server.
            //
            com.zeroc.Ice.ProcessPrx.uncheckedCast(
                communicator.stringToProxy("DemoIceBox/admin -f Process:default -p 9996")).shutdown();
        }
    }
}
