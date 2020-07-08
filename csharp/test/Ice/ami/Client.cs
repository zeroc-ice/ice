//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMI
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
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
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
