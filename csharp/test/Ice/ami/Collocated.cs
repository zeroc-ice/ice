// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.ami
{
    public class Collocated : TestHelper
    {
        public override async Task runAsync(string[] args)
        {
            Properties properties = createTestProperties(ref args);

            properties.setProperty("Ice.Warn.AMICallback", "0");

            // Limit the send buffer size, this test relies on the socket send() blocking after sending a given
            // amount of data.
            properties.setProperty("Ice.TCP.SndSize", "50000");

            // We use a client thread pool with more than one thread to test that task inlining works.
            properties.setProperty("Ice.ThreadPool.Client.Size", "5");
            using var communicator = initialize(properties);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(), Util.stringToIdentity("test"));
            adapter.add(new TestII(), Util.stringToIdentity("test2"));
            // Collocated test doesn't need to activate the OA
            adapter2.add(new TestControllerI(adapter), Util.stringToIdentity("testController"));
            // Collocated test doesn't need to activate the OA

            await AllTests.allTestsAsync(this, true);
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Collocated>(args);
    }
}
