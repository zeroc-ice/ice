//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.plugin.plugins;

public class PluginThreeFailFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginThreeFail(communicator);
    }

    public class PluginThreeFail extends BasePluginFail
    {
        public PluginThreeFail(com.zeroc.Ice.Communicator communicator)
        {
            super(communicator);
        }

        @Override
        public void initialize()
        {
            throw new PluginInitializeFailException();
        }

        @Override
        public void destroy()
        {
            test(false);
        }

        @SuppressWarnings("deprecation")
        @Override
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
