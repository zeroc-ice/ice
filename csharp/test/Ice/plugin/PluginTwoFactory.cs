// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class PluginTwoFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginTwo(communicator);
    }

    internal class PluginTwo : BasePlugin
    {
        public PluginTwo(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginOne");
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
