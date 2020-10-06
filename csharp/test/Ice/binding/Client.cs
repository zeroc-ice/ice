// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);

            // Under out-of-FDs conditions, the server might close the connection after it accepted it and
            // sent the connection validation message (the failure typically occurs when calling StartRead on
            // the transport).
            properties["Ice.Warn.Connections"] = "0";

            await using Communicator communicator = Initialize(properties);
            AllTests.Run(this);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
