// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginTwoFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginTwo(communicator);

        internal class PluginTwo : BasePlugin
        {
            public PluginTwo(Communicator communicator)
                : base(communicator)
            {
            }

            public override Task ActivateAsync(PluginActivationContext context, CancellationToken cancel)
            {
                var other = (BasePlugin?)Communicator.GetPlugin("PluginOne");
                TestHelper.Assert(other != null);
                Other = other;
                TestHelper.Assert(Other.IsInitialized());
                Initialized = true;
                return Task.CompletedTask;
            }

            public override async ValueTask DisposeAsync()
            {
                if (!Destroyed)
                {
                    Destroyed = true;
                    await base.DisposeAsync();
                    TestHelper.Assert(Other != null);
                }
            }
        }
    }
}
