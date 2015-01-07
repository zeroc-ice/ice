
// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/LoggerF.ice>
#include <Ice/BuiltinSequences.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A communicator plug-in. A plug-in generally adds a feature to a
 * communicator, such as support for a protocol.
 *
 * The communicator loads its plug-ins in two stages: the first stage
 * creates the plug-ins, and the second stage invokes {@link Plugin.initialize} on
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
 * Each communicator has a plug-in manager to administer the set of
 * plug-ins.
 *
 **/
local interface PluginManager
{
    /**
     *
     * Initialize the configured plug-ins. The communicator automatically initializes
     * the plug-ins by default, but an application may need to interact directly with
     * a plug-in prior to initialization. In this case, the application must set
     * <tt>Ice.InitPlugins=0</tt> and then invoke {@link #initializePlugins}
     * manually. The plug-ins are initialized in the order in which they are loaded.
     * If a plug-in raises an exception during initialization, the communicator
     * invokes destroy on the plug-ins that have already been initialized.
     *
     * @throws InitializationException Raised if the plug-ins have already been initialized.
     *
     **/
    void initializePlugins();

    /**
     *
     * Get a list of plugins installed.
     *
     * @return The names of the plugins installed.
     *
     * @see #getPlugin
     *
     **/
    StringSeq getPlugins();

    /**
     *
     * Obtain a plug-in by name.
     *
     * @param name The plug-in's name.
     *
     * @return The plug-in.
     *
     * @throws NotRegisteredException Raised if no plug-in is found with the given name.
     *
     **/
    Plugin getPlugin(string name);

    /**
     *
     * Install a new plug-in.
     *
     * @param name The plug-in's name.
     *
     * @param pi The plug-in.
     *
     * @throws AlreadyRegisteredException Raised if a plug-in already exists with the given name.
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

