//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

public class PluginOneFactory : Ice.IPluginFactory
{
    public Ice.IPlugin create(Ice.Communicator communicator, string name, string[] args) => new PluginOne(communicator);

    internal class PluginOne : BasePlugin
    {
        public PluginOne(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void Initialize()
        {
            BasePlugin? other = (BasePlugin?)_communicator.GetPlugin("PluginTwo");
            Debug.Assert(other != null);
            _other = other;
            test(!_other.isInitialized());
            _initialized = true;
        }

        public override void Destroy()
        {
            _destroyed = true;
            test(_other.isDestroyed());
        }
    }
}
