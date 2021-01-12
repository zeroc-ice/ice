// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginOneFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new PluginOneFail(communicator);

        internal class PluginOneFail : BasePluginFail
        {
            public PluginOneFail(Communicator communicator)
                : base(communicator)
            {
            }

            public override Task ActivateAsync(CancellationToken cancel)
            {
                var two = (BasePluginFail?)_communicator.Plugins.FirstOrDefault(p => p.Name == "PluginTwoFail").Plugin;
                TestHelper.Assert(two != null);
                _two = two;
                TestHelper.Assert(!_two.isInitialized());
                var three =
                    (BasePluginFail?)_communicator.Plugins.FirstOrDefault(p => p.Name == "PluginThreeFail").Plugin;
                TestHelper.Assert(three != null);
                _three = three;
                TestHelper.Assert(!_three.isInitialized());
                _initialized = true;
                return Task.CompletedTask;
            }

            public override ValueTask DisposeAsync()
            {
                GC.SuppressFinalize(this);
                TestHelper.Assert(_two != null && _two.isDestroyed());

                // Not destroyed because initialize fails.
                TestHelper.Assert(_three != null && !_three.isDestroyed());
                _destroyed = true;
                return default;
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
