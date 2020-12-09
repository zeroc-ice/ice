// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Linq;
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

            public override Task ActivateAsync(CancellationToken cancel)
            {
                var one = (BasePluginFail?)_communicator.Plugins.FirstOrDefault(p => p.Name == "PluginOneFail").Plugin;
                TestHelper.Assert(one != null);
                _one = one;
                TestHelper.Assert(_one.isInitialized());
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
                _destroyed = true;
                return default;
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
