//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

            public override void Destroy()
            {
                _destroyed = true;
                TestHelper.Assert(_other != null);
                TestHelper.Assert(!_other.isDestroyed());
            }
        }
    }
}
