//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Each communicator has a plug-in manager to administer the set of
 * plug-ins.
 **/
public interface PluginManager
{
    /**
     * Initialize the configured plug-ins. The communicator automatically initializes
     * the plug-ins by default, but an application may need to interact directly with
     * a plug-in prior to initialization. In this case, the application must set
     * <code>Ice.InitPlugins=0</code> and then invoke {@link #initializePlugins}
     * manually. The plug-ins are initialized in the order in which they are loaded.
     * If a plug-in raises an exception during initialization, the communicator
     * invokes destroy on the plug-ins that have already been initialized.
     * @throws InitializationException Raised if the plug-ins have already been initialized.
     **/
    void initializePlugins();

    /**
     * Get a list of plugins installed.
     * @return The names of the plugins installed.
     *
     * @see #getPlugin
     **/
    String[] getPlugins();

    /**
     * Obtain a plug-in by name.
     * @param name The plug-in's name.
     * @return The plug-in.
     * @throws NotRegisteredException Raised if no plug-in is found with the given name.
     **/
    Plugin getPlugin(String name);

    /**
     * Install a new plug-in.
     * @param name The plug-in's name.
     * @param pi The plug-in.
     * @throws AlreadyRegisteredException Raised if a plug-in already exists with the given name.
     **/
    void addPlugin(String name, Plugin pi);

    /**
     * Called when the communicator is being destroyed.
     **/
    void destroy();
}
