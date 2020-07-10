//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.ACM
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            // The default connect timeout is the ACM timeout if ConnectTimeout is not set.
            properties["Ice.ConnectTimeout"] = "10s";
            using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
