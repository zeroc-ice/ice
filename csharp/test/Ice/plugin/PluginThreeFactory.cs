//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

public class PluginThreeFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginThree(communicator);
    }

    internal class PluginThree : BasePlugin
    {
        public PluginThree(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            BasePlugin? other = (BasePlugin?)_communicator.GetPlugin("PluginTwo");
            Debug.Assert(other != null);
            _other = other;
            test(_other.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            _destroyed = true;
            test(!_other.isDestroyed());
        }
    }
}
