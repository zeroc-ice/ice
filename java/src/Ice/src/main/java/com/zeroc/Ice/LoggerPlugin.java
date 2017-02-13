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
 * Class to support custom loggers. Applications using a custom logger
 * instantiate a <code>LoggerPlugin</code> with a custom logger and
 * return the instance from their {@link PluginFactory} implementation.
 *
 * @see PluginFactory
 * @see Plugin
 **/
public class LoggerPlugin implements Plugin
{
    /**
     * Installs a custom logger for a communicator.
     *
     * @param communicator The communicator using the custom logger.
     * @param logger The custom logger for the communicator.
     **/
    public
    LoggerPlugin(Communicator communicator, Logger logger)
    {
        if(communicator == null)
        {
            PluginInitializationException ex = new PluginInitializationException();
            ex.reason = "Communicator cannot be null";
            throw ex;
        }

        if(logger == null)
        {
            PluginInitializationException ex = new PluginInitializationException();
            ex.reason = "Logger cannot be null";
            throw ex;
        }

        com.zeroc.IceInternal.Instance instance = com.zeroc.IceInternal.Util.getInstance(communicator);
        instance.setLogger(logger);
    }

    /**
     * Called by the Ice run time during communicator initialization. The derived class
     * can override this method to perform any initialization that might be required
     * by a custom logger.
     **/
    @Override
    public void
    initialize()
    {
    }

    /**
     * Called by the Ice run time when the communicator is destroyed. The derived class
     * can override this method to perform any finalization that might be required
     * by a custom logger.
     **/
    @Override
    public void
    destroy()
    {
    }
}
