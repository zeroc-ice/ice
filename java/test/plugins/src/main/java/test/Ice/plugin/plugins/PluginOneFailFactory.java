// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginOneFailFactory implements Ice.PluginFactory
{
    @Override
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginOneFail(communicator);
    }

    static class PluginOneFail extends BasePluginFail
    {
        public PluginOneFail(Ice.Communicator communicator)
        {
            super(communicator);
        }

        @Override
        public void initialize()
        {
            _two = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginTwoFail");
            test(!_two.isInitialized());
            _three = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginThreeFail");
            test(!_three.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy()
        {
            test(_two.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three.isDestroyed());
            _destroyed = true;
        }

        @Override
        protected void finalize() throws Throwable
        {
            try
            {
                if(!_initialized)
                {
                    System.out.println(getClass().getName() + " not initialized");
                }
                if(!_destroyed)
                {
                    System.out.println(getClass().getName() + " not destroyed");
                }
            }
            finally
            {
                super.finalize();
            }
        }
    }
}
