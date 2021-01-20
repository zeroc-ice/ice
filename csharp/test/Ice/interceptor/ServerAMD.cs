// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class ServerAMD : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new AsyncMyObject());
            await DispatchInterceptors.ActivateAsync(adapter);

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            string pluginPath =
                string.Format("msbuild/plugin/{0}/Plugin.dll",
                    Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)));

            await using var communicator = CreateCommunicator(
                ref args,
                new Dictionary<string, string>()
                {
                    {
                        "Ice.Plugin.DispatchPlugin",
                        $"{pluginPath }:ZeroC.Ice.Test.Interceptor.DispatchPluginFactory"
                    }
                });

            return await RunTestAsync<ServerAMD>(communicator, args);
        }
    }
}
