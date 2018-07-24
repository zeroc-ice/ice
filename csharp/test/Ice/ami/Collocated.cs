// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace ami
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                Ice.Properties properties = createTestProperties(ref args);

                properties.setProperty("Ice.Warn.AMICallback", "0");
                //
                // Limit the send buffer size, this test relies on the socket
                // send() blocking after sending a given amount of data.
                //
                properties.setProperty("Ice.TCP.SndSize", "50000");
                //
                // We use a client thread pool with more than one thread to test
                // that task inlining works.
                //
                properties.setProperty("Ice.ThreadPool.Client.Size", "5");
                properties.setProperty("Ice.Package.Test", "Ice.ami");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                    communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                    adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
                    adapter.add(new TestII(), Ice.Util.stringToIdentity("test2"));
                    //adapter.activate(); // Collocated test doesn't need to activate the OA
                    adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
                    //adapter2.activate(); // Collocated test doesn't need to activate the OA

                    AllTests.allTests(this, true);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
