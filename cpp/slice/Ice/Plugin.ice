// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PLUGIN_ICE
#define ICE_PLUGIN_ICE

#include <Ice/LocalException.ice>

module Ice
{

/**
 *
 * A Communicator plug-in. A plug-in generally adds a feature to a
 * Communicator, such as support for a protocol.
 *
 **/
local interface Plugin
{
    /**
     *
     * Called when the Communicator is being destroyed.
     *
     **/
    void destroy();
};

/**
 *
 * Each Communicator has a PluginManager to administer the set of
 * plug-ins.
 *
 **/
local interface PluginManager
{
    /**
     *
     * Obtain a plug-in by name.
     *
     * @param name The plug-in's name.
     *
     * @return The plug-in.
     *
     * @throws PluginNotFoundException Raised if no plug-in was
     * found with the given name.
     *
     **/
    Plugin getPlugin(string name)
        throws PluginNotFoundException;

    /**
     *
     * Install a new plug-in.
     *
     * @param name The plug-in's name.
     *
     * @param pi The plug-in.
     *
     * @throws PluginExistsException Raised if a plug-in already
     * exists with the given name.
     *
     **/
    void addPlugin(string name, Plugin pi)
        throws PluginExistsException;

    /**
     *
     * Called when the Communicator is being destroyed.
     *
     **/
    void destroy();
};

};

#endif
