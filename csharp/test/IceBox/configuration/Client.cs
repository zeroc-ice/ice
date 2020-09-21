// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Configuration
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string>? properties = CreateTestProperties(ref args);
            await using Communicator communicator = Initialize(properties);
            AllTests.Run(this);
            // Shutdown the IceBox server.
            await IProcessPrx.Parse(Protocol == Protocol.Ice1 ?
                                    $"DemoIceBox/admin -f Process:{Transport} -h 127.0.0.1 -p 9996" :
                                    $"ice+{Transport}://127.0.0.1:9996/DemoIceBox/admin#Process",
                                    communicator).ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
