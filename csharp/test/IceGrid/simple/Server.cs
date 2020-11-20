// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceGrid.Test.Simple
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = new Dictionary<string, string>();
            properties.ParseArgs(ref args, "TestAdapter");

            // TODO: temporary until IceGrid is fixed the generate 127.0.0.1
            properties.ParseArgs(ref args);
            if (properties.TryGetValue("Ice.Admin.Endpoints", out string? value) && value.Contains("localhost"))
            {
                properties["Ice.Admin.Endpoints"] = value.Replace("localhost", "127.0.0.1");
                properties["Ice.ServerName"] = "127.0.0.1";
            }

            await using Communicator communicator = Initialize(ref args, properties);
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(communicator.GetProperty("Identity") ?? "test", new TestIntf());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
