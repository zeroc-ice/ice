//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Class to support thread notification hooks. Applications using thread
 * notifications hooks instantiate a <code>ThreadHookPlugin</code> with
 * a thread start and threaed stop Runnable and return the instance from their
 * {@link PluginFactory} implementation.
 *
 * @see PluginFactory
 * @see Plugin
 **/
public class ThreadHookPlugin implements Plugin
{
    /**
     * Installs thread notification hooks for a communicator.
     *
     * @param communicator The communicator using the thread notification hooks.
     * @param threadStart The callback for newly started threads.
     * @param threadStop The callback for stopped threads.
     **/
    public
    ThreadHookPlugin(Communicator communicator, Runnable threadStart, Runnable threadStop)
    {
        if(communicator == null)
        {
            PluginInitializationException ex = new PluginInitializationException();
            ex.reason = "Communicator cannot be null";
            throw ex;
        }

        com.zeroc.IceInternal.Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        instance.setThreadHooks(threadStart, threadStop);
    }

    /**
     * Called by the Ice run time during communicator initialization. The derived class
     * can override this method to perform any initialization that might be required
     * by custom thread notification hooks.
     **/
    @Override
    public void
    initialize()
    {
    }

    /**
     * Called by the Ice run time when the communicator is destroyed. The derived class
     * can override this method to perform any finalization that might be required
     * by custom thread notification hooks.
     **/
    @Override
    public void
    destroy()
    {
    }
}
