//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);

            // Under out-of-FDs condititions, the server might close the connection after it accepted it and
            // sent the connection validation message (the failure typically occurs when calling StartRead on
            // the transport).
            properties["Ice.Warn.Connections"] = "0";

            using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
