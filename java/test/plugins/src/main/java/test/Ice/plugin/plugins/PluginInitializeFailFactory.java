// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginInitializeFailFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginInitializeFail();
    }

    static class PluginInitializeFail implements com.zeroc.Ice.Plugin
    {
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

        private static void test(boolean b)
        {
            if(!b)
            {
                throw new RuntimeException();
            }
        }
    }
}
