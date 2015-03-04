// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

public class PluginFactoryI implements Ice.PluginFactory
{
    static class PluginI implements Ice.Plugin
    {
        public void initialize()
        {
        }

        public void destroy()
        {
        }
    }

    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI();
    }
}
