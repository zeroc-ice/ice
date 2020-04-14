//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

public class PluginTwoFailFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) =>
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
            Trace.Assert(_one.isInitialized());
            BasePluginFail? three = (BasePluginFail?)_communicator.GetPlugin("PluginThreeFail");
            Debug.Assert(three != null);
            _three = three;
            Trace.Assert(!_three.isInitialized());
            _initialized = true;
        }

        public override void Destroy()
        {
            Trace.Assert(_one != null && !_one.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            Trace.Assert(_three != null && !_three.isDestroyed());
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
