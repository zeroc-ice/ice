// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.classLoader;

public class PluginFactoryI implements Ice.PluginFactory
{
    static class PluginI implements Ice.Plugin
    {
        @Override
        public void initialize()
        {
        }

        @Override
        public void destroy()
        {
        }
    }

    @Override
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI();
    }
}
