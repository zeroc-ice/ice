//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.ami
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            System.Collections.Generic.Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.AMICallback"] = "0";
            properties["Ice.Warn.Connections"] = "0";

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            properties["Ice.TCP.SndSize"] = "50000";
            using Communicator communicator = Initialize(properties);
            AllTests.allTests(this, false);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
