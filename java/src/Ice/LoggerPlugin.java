// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class LoggerPlugin implements Ice.Plugin
{
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

        IceInternal.Instance instance = Util.getInstance(communicator);
        instance.setLogger(logger);
    }

    public void 
    initialize()
    {
    }

    public void
    destroy()
    {
    }
}

