// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PLUGIN_ICE
#define ICE_PLUGIN_ICE

module Ice
{

/**
 *
 * A communicator plugin. A plugin generally adds a feature to a
 * communicator, such as support for a protocol.
 *
 * The communicator loads its plugins in two stages: the first stage
 * creates the plugins, and the second stage invokes [initialize] on
 * each one.
 *
 **/
local interface Plugin
{
    /**
     *
     * Perform any necessary initialization steps.
     *
     **/
    void initialize();

    /**
     *
     * Called when the communicator is being destroyed.
     *
     **/
    void destroy();
};

/**
 *
 * Each communicator has a plugin manager to administer the set of
 * plugins.
 *
 **/
local interface PluginManager
{
    /**
     *
     * Initialize the configured plugins. The communicator automatically initializes
     * the plugins by default, but an application may need to interact directly with
     * a plugin prior to initialization. In this case, the application must set
     * <tt>Ice.InitPlugins=0</tt> and then invoke [initializePlugins]
     * manually. The plugins are initialized in the order in which they are loaded.
     * If a plugin raises an exception during initialization, the communicator
     * invokes destroy on the plugins that have already been initialized.
     *
     * @throws InitializationException Raised if the plugins have already been initialized.
     *
     **/
    void initializePlugins();

    /**
     *
     * Obtain a plugin by name.
     *
     * @param name The plugin's name.
     *
     * @return The plugin.
     *
     * @throws NotRegisteredException Raised if no plugin is found with the given name.
     *
     **/
    Plugin getPlugin(string name);

    /**
     *
     * Install a new plugin.
     *
     * @param name The plugin's name.
     *
     * @param pi The plugin.
     *
     * @throws AlreadyRegisteredException Raised if a plugin already exists with the given name.
     *
     **/
    void addPlugin(string name, Plugin pi);

    /**
     *
     * Called when the communicator is being destroyed.
     *
     **/
    void destroy();
};

};

#endif
