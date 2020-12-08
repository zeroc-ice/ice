// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class ServerAMD : TestHelper
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
                        "Ice.Plugin.DispatchPlugin",
                        $"{pluginPath }:ZeroC.Ice.Test.Interceptor.DispatchPluginFactory"
                    }
                });

            await communicator.ActivateAsync();

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new AsyncMyObject());
            await DispatchInterceptors.ActivateAsync(adapter);
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<ServerAMD>(args);
    }
}
