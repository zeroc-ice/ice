//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

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
            var one = (BasePluginFail?)_communicator.GetPlugin("PluginOneFail");
            TestHelper.Assert(one != null);
            _one = one;
            TestHelper.Assert(_one.isInitialized());
            var three = (BasePluginFail?)_communicator.GetPlugin("PluginThreeFail");
            TestHelper.Assert(three != null);
            _three = three;
            TestHelper.Assert(!_three.isInitialized());
            _initialized = true;
        }

        public override void Destroy()
        {
            TestHelper.Assert(_one != null && !_one.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            TestHelper.Assert(_three != null && !_three.isDestroyed());
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
