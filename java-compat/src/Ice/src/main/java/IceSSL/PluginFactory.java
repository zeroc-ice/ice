// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceSSL;

/**
 * Plug-in factories must implement this interface.
 **/
public class PluginFactory implements Ice.PluginFactory
{
    /**
     * Returns a new plug-in.
     *
     * @param communicator The communicator for the plug-in.
     * @param name The name of the plug-in.
     * @param args The arguments that are specified in the plug-in's configuration.
     *
     * @return The new plug-in. <code>null</code> can be returned to indicate
     * that a general error occurred. Alternatively, <code>create</code> can throw
     * PluginInitializationException to provide more detailed information.
     **/
    @Override
    public Ice.Plugin
    create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
