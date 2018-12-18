// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public abstract class BasePluginFail implements com.zeroc.Ice.Plugin
{
    public BasePluginFail(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
        _initialized = false;
        _destroyed = false;
    }

    public boolean isInitialized()
    {
        return _initialized;
    }

    public boolean isDestroyed()
    {
        return _destroyed;
    }

    protected static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    protected com.zeroc.Ice.Communicator _communicator;
    protected boolean _initialized;
    protected boolean _destroyed;
    protected BasePluginFail _one;
    protected BasePluginFail _two;
    protected BasePluginFail _three;
}
