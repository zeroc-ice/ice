//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginOneFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginOne(communicator);

        internal class PluginOne : BasePlugin
        {
            public PluginOne(Communicator communicator) : base(communicator)
            {
            }

            public override void Initialize()
            {
                var other = (BasePlugin?)_communicator.GetPlugin("PluginTwo");
                TestHelper.Assert(other != null);
                _other = other;
                TestHelper.Assert(!_other.isInitialized());
                _initialized = true;
            }

            public override ValueTask DisposeAsync()
            {
                _destroyed = true;
                TestHelper.Assert(_other!.isDestroyed());
                return new ValueTask(Task.CompletedTask);
            }
        }
    }
}
