//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

public class PluginTwoFailFactory : Ice.IPluginFactory
{
    public Ice.IPlugin create(Ice.Communicator communicator, string name, string[] args) =>
        new PluginTwoFail(communicator);

    internal class PluginTwoFail : BasePluginFail
    {
        public PluginTwoFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void Initialize()
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

        public override void Destroy()
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
