// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public abstract class BasePlugin : Ice.Plugin
{
    public BasePlugin(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public bool isInitialized()
    {
        return _initialized;
    }

    public bool isDestroyed()
    {
        return _destroyed;
    }

    protected static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public abstract void initialize();
    public abstract void destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized = false;
    protected bool _destroyed = false;
    protected BasePlugin _other = null;
}
