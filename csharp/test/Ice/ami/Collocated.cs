//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

using Ice.ami.Test;
using Ice.ami.Test.Outer.Inner;

namespace Ice.ami
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);

            properties["Ice.Warn.AMICallback"] = "0";
            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            properties["Ice.TCP.SndSize"] = "50000";

            using var communicator = Initialize(properties);

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            //adapter.activate(); // Collocated test doesn't need to activate the OA

            AllTests.allTests(this, true);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
