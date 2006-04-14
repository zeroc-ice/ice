// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
 **/
local interface Plugin
{
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
