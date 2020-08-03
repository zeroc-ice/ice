//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Compress
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            TextWriter output = GetWriter();
            var properties = CreateTestProperties(ref args);
            output.Write("testing operations using compression... ");
            output.Flush();
            {
                properties["Ice.CompressionMinSize"] = "1K";
                await using Communicator communicator = Initialize(properties);
                _ = AllTests.Run(this, communicator);
            }

            {
                // Repeat with Optimal compression level
                properties["Ice.CompressionLevel"] = "Optimal";
                await using Communicator communicator = Initialize(properties);
                ITestIntfPrx? server = AllTests.Run(this, communicator);
                await server.ShutdownAsync();
            }

            output.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
