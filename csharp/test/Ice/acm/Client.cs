//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

namespace Ice
{
    namespace acm
    {
        public class Client : TestHelper
        {
            public override void Run(string[] args)
            {
                Dictionary<string, string> properties = CreateTestProperties(ref args);
                properties["Ice.Warn.Connections"] = "0";
                using Communicator communicator = Initialize(properties);
                AllTests.allTests(this);
            }

            public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
        }
    }
}
