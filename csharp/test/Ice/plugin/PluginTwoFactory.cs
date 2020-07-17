//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginTwoFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginTwo(communicator);

        internal class PluginTwo : BasePlugin
        {
            public PluginTwo(Communicator communicator) : base(communicator)
            {
            }

            public override void Initialize()
            {
                var other = (BasePlugin?)_communicator.GetPlugin("PluginOne");
                TestHelper.Assert(other != null);
                _other = other;
                TestHelper.Assert(_other.isInitialized());
                _initialized = true;
            }

            public override async ValueTask DisposeAsync()
            {
                if (!_destroyed)
                {
                    _destroyed = true;
                    await base.DisposeAsync();
                    TestHelper.Assert(_other != null);
                }
            }
        }
    }
}
