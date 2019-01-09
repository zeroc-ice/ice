// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginThreeFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginThree(communicator);
    }

    static class PluginThree extends BasePlugin
    {
        public PluginThree(com.zeroc.Ice.Communicator communicator)
        {
            super(communicator);
        }

        @Override
        public void initialize()
        {
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginTwo");
            test(_other.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy()
        {
            _destroyed = true;
            test(!_other.isDestroyed());
        }
    }
}
