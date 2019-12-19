//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

public class PluginOneFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginOneFail(communicator);
    }

    internal class PluginOneFail : BasePluginFail
    {
        public PluginOneFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            BasePluginFail? two = (BasePluginFail?)_communicator.GetPlugin("PluginTwoFail");
            Debug.Assert(two != null);
            _two = two;
            test(!_two.isInitialized());
            BasePluginFail? three = (BasePluginFail?)_communicator.GetPlugin("PluginThreeFail");
            Debug.Assert(three != null);
            _three = three;
            test(!_three.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            test(_two.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three.isDestroyed());
            _destroyed = true;
        }

        ~PluginOneFail()
        {
            if (!_initialized)
            {
                Console.WriteLine("PluginOneFail not initialized");
            }
            if (!_destroyed)
            {
                Console.WriteLine("PluginOneFail not destroyed");
            }
        }
    }
}
