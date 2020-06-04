//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginOneFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new PluginOneFail(communicator);

        internal class PluginOneFail : BasePluginFail
        {
            public PluginOneFail(Communicator communicator) : base(communicator)
            {
            }

            public override void Initialize()
            {
                var two = (BasePluginFail?)_communicator.GetPlugin("PluginTwoFail");
                TestHelper.Assert(two != null);
                _two = two;
                TestHelper.Assert(!_two.isInitialized());
                var three = (BasePluginFail?)_communicator.GetPlugin("PluginThreeFail");
                TestHelper.Assert(three != null);
                _three = three;
                TestHelper.Assert(!_three.isInitialized());
                _initialized = true;
            }

            public override void Destroy()
            {
                TestHelper.Assert(_two != null && _two.isDestroyed());
                //
                // Not destroyed because initialize fails.
                //
                TestHelper.Assert(_three != null && !_three.isDestroyed());
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
}
