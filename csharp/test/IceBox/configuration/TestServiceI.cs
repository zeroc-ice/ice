// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceBox.Test.Configuration
{
    public class TestService : IService
    {
        public async Task StartAsync(
            string name,
            Communicator communicator,
            string[] args,
            CancellationToken cancel)
        {
            ObjectAdapter adapter = communicator.CreateObjectAdapter(name + "OA");
            adapter.Add("test", new TestIntf(args));
            await adapter.ActivateAsync(cancel);
        }

        public Task StopAsync() => Task.CompletedTask;
    }
}
