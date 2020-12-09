// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginOneFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginOne(communicator);

        internal class PluginOne : BasePlugin
        {
            public PluginOne(Communicator communicator)
                : base(communicator)
            {
            }

            public override Task ActivateAsync(CancellationToken cancel)
            {
                var other = (BasePlugin?)Communicator.Plugins.FirstOrDefault(p => p.Name == "PluginTwo").Plugin;
                TestHelper.Assert(other != null);
                Other = other;
                TestHelper.Assert(!Other.IsInitialized());
                Initialized = true;
                return Task.CompletedTask;
            }

            public override async ValueTask DisposeAsync()
            {
                if (!Destroyed)
                {
                    await base.DisposeAsync();
                    Destroyed = true;
                }
            }
        }
    }
}
