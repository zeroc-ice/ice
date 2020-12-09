// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginThreeFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new PluginThreeFail(communicator);

        internal class PluginThreeFail : BasePluginFail
        {
            public PluginThreeFail(Communicator communicator)
                : base(communicator)
            {
            }

            public override Task ActivateAsync(CancellationToken cancel) =>
                throw new PluginInitializeFailException();

            public override ValueTask DisposeAsync()
            {
                GC.SuppressFinalize(this);
                return default;
            }

            ~PluginThreeFail()
            {
                if (_initialized)
                {
                    Console.WriteLine("PluginThreeFail was initialized");
                }
                if (_destroyed)
                {
                    Console.WriteLine("PluginThreeFail was destroyed");
                }
            }
        }
    }
}
