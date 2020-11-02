// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Assemblies
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Console.Out.Write("testing preloading assemblies... ");
            Console.Out.Flush();
            var info = new User.UserInfo();

            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.PreloadAssemblies"] = "0";

            string assembly =
                Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!, "core.dll");

            await using (Communicator communicator = Initialize(properties))
            {
                Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.Location.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) == null);
            }
            properties["Ice.PreloadAssemblies"] = "1";
            await using (Communicator communicator = Initialize(properties))
            {
                Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.Location.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) != null);
            }

            Console.Out.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
