// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginThreeFailFactory implements Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginThreeFail(communicator);
    }

    public class PluginThreeFail extends BasePluginFail
    {
        public PluginThreeFail(Ice.Communicator communicator)
        {
            super(communicator);
        }

        public void initialize()
        {
            throw new PluginInitializeFailException();
        }

        public void destroy()
        {
            test(false);
        }

        protected void finalize() throws Throwable
        {
            try
            {
                if(_initialized)
                {
                    System.out.println(getClass().getName() + " was initialized");
                }
                if(_destroyed)
                {
                    System.out.println(getClass().getName() + " was destroyed");
                }
            }
            finally
            {
                super.finalize();
            }
        }
    }
}
