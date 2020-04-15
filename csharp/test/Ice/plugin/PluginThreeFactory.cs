//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class PluginThreeFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new PluginThree(communicator);

    internal class PluginThree : BasePlugin
    {
        public PluginThree(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void Initialize()
        {
            var other = (BasePlugin?)_communicator.GetPlugin("PluginTwo");
            TestHelper.Assert(other != null);
            _other = other;
            TestHelper.Assert(_other.isInitialized());
            _initialized = true;
        }

        public override void Destroy()
        {
            _destroyed = true;
            TestHelper.Assert(_other != null && !_other.isDestroyed());
        }
    }
}
