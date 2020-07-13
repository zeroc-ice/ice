//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using Test;
using System.Threading.Tasks;

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
                $"{Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase))}/core.dll";

            await using (Communicator communicator = Initialize(properties))
            {
                Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.CodeBase!.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) == null);
            }
            properties["Ice.PreloadAssemblies"] = "1";
            await using (Communicator communicator = Initialize(properties))
            {
                Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                    e => e.CodeBase!.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) != null);
            }

            Console.Out.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
