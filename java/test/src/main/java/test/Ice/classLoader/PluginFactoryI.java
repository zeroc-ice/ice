// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

public class PluginFactoryI implements com.zeroc.Ice.PluginFactory
{
    static class PluginI implements com.zeroc.Ice.Plugin
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
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI();
    }
}
