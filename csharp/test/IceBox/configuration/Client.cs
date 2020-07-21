//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;
using Test;
using System.Threading.Tasks;

namespace ZeroC.IceBox.Test.Configuration
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Default.Host"] = "127.0.0.1";
            await using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
            // Shutdown the IceBox server.
            string transport = communicator.DefaultTransport.ToString().ToLowerInvariant();

            IProcessPrx.Parse(communicator.DefaultProtocol == Protocol.Ice1 ?
                                $"DemoIceBox/admin -f Process:{transport} -h 127.0.0.1 -p 9996" :
                                $"ice+{transport}://127.0.0.1:9996/DemoIceBox/admin#Process",
                              communicator).Shutdown();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
