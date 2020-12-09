// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginInitializeFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginInitializeFail();

        internal class PluginInitializeFail : IPlugin
        {
            public Task ActivateAsync(CancellationToken cancel) =>
                throw new PluginInitializeFailException();

            public ValueTask DisposeAsync() => default;
        }
    }
}
