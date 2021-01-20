// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Assemblies
{
    public static class Client
    {
        public static async Task RunAsync(string[] args)
        {
            Console.Out.Write("testing preloading assemblies... ");
            Console.Out.Flush();
            var info = new User.UserInfo();

            Dictionary<string, string> properties = TestHelper.CreateTestProperties(ref args);
            string assembly =
                Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!, "core.dll");

            properties["Ice.PreloadAssemblies"] = "0";
            await using (Communicator communicator = TestHelper.CreateCommunicator(properties))
            {
                TestHelper.Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.Location.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) == null);
            }

            properties["Ice.PreloadAssemblies"] = "1";
            await using (Communicator communicator = TestHelper.CreateCommunicator(properties))
            {
                TestHelper.Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.Location.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) != null);
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
