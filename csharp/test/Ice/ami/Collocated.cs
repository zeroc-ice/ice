//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

using Ice.ami.Test;
using Ice.ami.Test.Outer;
using Ice.ami.Test.Outer.Inner;

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
                using (var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                    communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                    adapter.Add(new TestI(), "test");
                    adapter.Add(new TestII(), "test2");
                    //adapter.activate(); // Collocated test doesn't need to activate the OA
                    adapter2.Add(new TestControllerI(adapter), "testController");
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
