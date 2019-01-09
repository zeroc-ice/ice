// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;

public class PluginTwoFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginTwoFail(communicator);
    }

    internal class PluginTwoFail : BasePluginFail
    {
        public PluginTwoFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            _one = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginOneFail");
            test(_one.isInitialized());
            _three = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginThreeFail");
            test(!_three.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            test(!_one.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three.isDestroyed());
            _destroyed = true;
        }

        ~PluginTwoFail()
        {
            if(!_initialized)
            {
                Console.WriteLine("PluginTwoFail not initialized");
            }
            if(!_destroyed)
            {
                Console.WriteLine("PluginTwoFail not destroyed");
            }
        }
    }
}
