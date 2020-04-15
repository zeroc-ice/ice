//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class PluginTwoFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new PluginTwo(communicator);

    internal class PluginTwo : BasePlugin
    {
        public PluginTwo(Ice.Communicator communicator) : base(communicator)
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
