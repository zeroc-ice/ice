// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public abstract class BasePlugin implements com.zeroc.Ice.Plugin
{
    public BasePlugin(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
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
    protected boolean _initialized = false;
    protected boolean _destroyed = false;
    protected BasePlugin _other = null;
}
