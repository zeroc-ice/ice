//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class PluginThreeFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginThreeFail(communicator);
    }

    internal class PluginThreeFail : BasePluginFail
    {
        public PluginThreeFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            throw new PluginInitializeFailException();
        }

        public override void destroy()
        {
            test(false);
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
