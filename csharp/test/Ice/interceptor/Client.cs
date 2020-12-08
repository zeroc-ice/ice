// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            string pluginPath =
                string.Format("msbuild/plugin/{0}/Plugin.dll",
                    Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)));
            await using Communicator communicator = Initialize(
                ref args,
                new Dictionary<string, string>()
                {
                    {
                        "Ice.Plugin.InvocationPlugin",
                        $"{pluginPath}:ZeroC.Ice.Test.Interceptor.InvocationPluginFactory"
                    }
                });

            await communicator.ActivateAsync();

            IMyObjectPrx prx = AllTests.Run(this);
            await prx.ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
