//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

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
            BasePluginFail? one = (BasePluginFail?)_communicator.GetPlugin("PluginOneFail");
            Debug.Assert(one != null);
            _one = one;
            test(_one.isInitialized());
            BasePluginFail? three = (BasePluginFail?)_communicator.GetPlugin("PluginThreeFail");
            Debug.Assert(three != null);
            _three = three;
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
            if (!_initialized)
            {
                Console.WriteLine("PluginTwoFail not initialized");
            }
            if (!_destroyed)
            {
                Console.WriteLine("PluginTwoFail not destroyed");
            }
        }
    }
}
