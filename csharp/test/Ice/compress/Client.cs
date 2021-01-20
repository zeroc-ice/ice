// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Compress
{
    public static class Client
    {
        public static async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = TestHelper.CreateTestProperties(ref args);
            Console.Out.Write("testing operations using compression... ");
            Console.Out.Flush();

            {
                properties["Ice.CompressionMinSize"] = "1K";
                await using var communicator = TestHelper.CreateCommunicator(properties);
                await AllTests.RunAsync(communicator, false);
            }

            {
                // Repeat with Optimal compression level
                properties["Ice.CompressionLevel"] = "Optimal";
                await using var communicator = TestHelper.CreateCommunicator(properties);
                await AllTests.RunAsync(communicator, true);
            }

            Console.Out.WriteLine("ok");
        }

        public static async Task<int> Main(string[] args)
        {
            int status = 0;
            try
            {
                await RunAsync(args);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return status;
        }
    }
}
