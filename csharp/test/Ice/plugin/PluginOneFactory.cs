//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class PluginOneFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new PluginOne(communicator);

    internal class PluginOne : BasePlugin
    {
        public PluginOne(Ice.Communicator communicator) : base(communicator)
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

        public override void Destroy()
        {
            _destroyed = true;
            TestHelper.Assert(_other!.isDestroyed());
        }
    }
}
