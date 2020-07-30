//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
            string transport = communicator.DefaultTransport.ToString().ToLowerInvariant();

            await IProcessPrx.Parse(communicator.DefaultProtocol == Protocol.Ice1 ?
                                    $"DemoIceBox/admin -f Process:{transport} -h 127.0.0.1 -p 9996" :
                                    $"ice+{transport}://127.0.0.1:9996/DemoIceBox/admin#Process",
                                    communicator).ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
