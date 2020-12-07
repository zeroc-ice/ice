// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginTwoFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new PluginTwoFail(communicator);

        internal class PluginTwoFail : BasePluginFail
        {
            public PluginTwoFail(Communicator communicator)
                : base(communicator)
            {
            }

            public override Task ActivateAsync(PluginActivationContext context, CancellationToken cancel)
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
                return Task.CompletedTask;
            }

            public override ValueTask DisposeAsync()
            {
                GC.SuppressFinalize(this);
                TestHelper.Assert(_one != null && !_one.isDestroyed());

                // Not destroyed because initialize fails.
                TestHelper.Assert(_three != null && !_three.isDestroyed());
                _destroyed = true;
                return new ValueTask(Task.CompletedTask);
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
}
