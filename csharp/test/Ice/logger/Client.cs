// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Logger
{
    public static class Client
    {
        public static async Task RunAsync(string[] args)
        {
            Console.Out.Write("testing Ice.LogFile... ");
            Console.Out.Flush();
            if (File.Exists("log.txt"))
            {
                File.Delete("log.txt");
            }

            {
                var properties = TestHelper.CreateTestProperties(ref args);
                properties["Ice.LogFile"] = "log.txt";
                await using var communicator = TestHelper.CreateCommunicator(properties);
                communicator.Logger.Trace("info", "my logger");
            }
            TestHelper.Assert(File.Exists("log.txt"));
            TestHelper.Assert(File.ReadAllText("log.txt").Contains("my logger"));
            File.Delete("log.txt");
            Console.Out.WriteLine("ok");
        }

        public static async Task<int> Main(string[] args)
        {
            try
            {
                await RunAsync(args);
                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
            return 1;
        }
    }
}
