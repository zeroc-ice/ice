//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginThreeFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new PluginThreeFail(communicator);

        internal class PluginThreeFail : BasePluginFail
        {
            public PluginThreeFail(Communicator communicator) : base(communicator)
            {
            }

            public override void Initialize() => throw new PluginInitializeFailException();

            public override ValueTask DisposeAsync()
            {
                TestHelper.Assert(false);
                return new ValueTask(Task.CompletedTask);
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
