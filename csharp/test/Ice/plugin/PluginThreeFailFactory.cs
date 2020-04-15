//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class PluginThreeFailFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new PluginThreeFail(communicator);

    internal class PluginThreeFail : BasePluginFail
    {
        public PluginThreeFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void Initialize() => throw new PluginInitializeFailException();

        public override void Destroy() => TestHelper.Assert(false);

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
