// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            if(_initialized)
            {
                Console.WriteLine("PluginThreeFail was initialized");
            }
            if(_destroyed)
            {
                Console.WriteLine("PluginThreeFail was destroyed");
            }
        }
    }
}
