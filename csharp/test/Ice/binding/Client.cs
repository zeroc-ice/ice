//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace Ice.binding
{
    public class Client : TestHelper
    {
        public override void run(string[] args)
        {
            Dictionary<string, string> properties = createTestProperties(ref args);

            // Under out-of-FDs condititions, the server might close the connection after it accepted it and
            // sent the connection validation message (the failure typically occurs when calling StartRead on
            // the transport).
            properties["Ice.Warn.Connections"] = "0";

            using var communicator = initialize(properties);
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Client>(args);
    }
}
