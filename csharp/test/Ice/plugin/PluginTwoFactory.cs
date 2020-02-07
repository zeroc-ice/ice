//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System.Diagnostics;

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
            BasePlugin? other = (BasePlugin?)_communicator.GetPlugin("PluginOne");
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
