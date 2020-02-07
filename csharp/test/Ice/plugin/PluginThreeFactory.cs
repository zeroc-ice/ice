//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

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
            BasePlugin? other = (BasePlugin?)_communicator.GetPlugin("PluginTwo");
            Debug.Assert(other != null);
            _other = other;
            test(_other.isInitialized());
            _initialized = true;
        }

        public override void Destroy()
        {
            _destroyed = true;
            test(!_other.isDestroyed());
        }
    }
}
