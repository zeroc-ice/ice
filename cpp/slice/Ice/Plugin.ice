// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PLUGIN_ICE
#define ICE_PLUGIN_ICE

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
 * A plug-in already exists with the same name.
 *
 **/
local exception PluginExistsException
{
};

/**
 *
 * No plug-in exists with the given name.
 *
 **/
local exception PluginNotFoundException
{
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
     * @param plugin The plug-in.
     *
     * @throws PluginExistsException Raised if a plug-in already
     * exists with the given name.
     *
     **/
    void addPlugin(string name, Plugin plugin)
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
