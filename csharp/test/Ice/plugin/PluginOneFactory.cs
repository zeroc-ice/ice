// Copyright (c) ZeroC, Inc. All rights reserved.

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

            public override void Initialize(PluginInitializationContext context)
            {
                var other = (BasePlugin?)Communicator.GetPlugin("PluginTwo");
                TestHelper.Assert(other != null);
                Other = other;
                TestHelper.Assert(!Other.IsInitialized());
                Initialized = true;
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
