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
            output.Write("testing operations using compression... ");
            output.Flush();
            {
                await using Communicator communicator = Initialize(ref args,
                    new Dictionary<string, string>()
                    {
                        ["Ice.CompressionMinSize"] = "1K"
                    });
                    _ = AllTests.Run(this, communicator);
            }

            {
                // Repeat with Optimal compression level
                await using Communicator communicator = Initialize(ref args,
                    new Dictionary<string, string>()
                    {
                        ["Ice.CompressionLevel"] = "Optimal",
                        ["Ice.CompressionMinSize"] = "1K"
                    });
                ITestIntfPrx? server = AllTests.Run(this, communicator);
                await server.ShutdownAsync();
            }

            output.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
