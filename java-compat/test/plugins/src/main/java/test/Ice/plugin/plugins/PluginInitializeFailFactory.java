//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.plugin.plugins;

public class PluginInitializeFailFactory implements Ice.PluginFactory
{
    @Override
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginInitializeFail();
    }

    static class PluginInitializeFail implements Ice.Plugin
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
