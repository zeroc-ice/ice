// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginTwo");
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
