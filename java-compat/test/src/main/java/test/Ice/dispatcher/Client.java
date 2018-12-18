// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.dispatcher;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Dispatcher dispatcher = new Dispatcher();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = createTestProperties(args);
            initData.properties.setProperty("Ice.Package.Test", "test.Ice.dispatcher");

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            initData.properties.setProperty("Ice.TCP.SndSize", "50000");
            initData.dispatcher = dispatcher;
            try(Ice.Communicator communicator = initialize(initData))
            {
                AllTests.allTests(this, dispatcher);
            }
        }
        finally
        {
            dispatcher.terminate();
        }
    }
}
