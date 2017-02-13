// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Class to support thread notification hooks. Applications using thread
 * notifications hooks instantiate a <code>ThreadHookPlugin</code> with
 * a thread notification hook and return the instance from their
 * {@link PluginFactory} implementation.
 *
 * @see PluginFactory
 * @see Plugin
 **/
public class ThreadHookPlugin implements Plugin
{
    /**
     * Installs a thread notification hook for a communicator.
     *
     * @param communicator The communicator using the thread notification hook.
     * @param threadHook The thread notification hook for the communicator.
     **/
    public
    ThreadHookPlugin(Communicator communicator, ThreadNotification threadHook)
    {
        if(communicator == null)
        {
            PluginInitializationException ex = new PluginInitializationException();
            ex.reason = "Communicator cannot be null";
            throw ex;
        }

        com.zeroc.IceInternal.Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        instance.setThreadHook(threadHook);
    }

    /**
     * Called by the Ice run time during communicator initialization. The derived class
     * can override this method to perform any initialization that might be required
     * by a custom thread notification hook.
     **/
    @Override
    public void
    initialize()
    {
    }

    /**
     * Called by the Ice run time when the communicator is destroyed. The derived class
     * can override this method to perform any finalization that might be required
     * by a custom thread notification hook.
     **/
    @Override
    public void
    destroy()
    {
    }
}
