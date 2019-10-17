
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#include <Ice/LoggerF.ice>
#include <Ice/BuiltinSequences.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

#if !defined(__SLICE2PHP__)
/**
 *
 * A communicator plug-in. A plug-in generally adds a feature to a
 * communicator, such as support for a protocol.
 *
 * The communicator loads its plug-ins in two stages: the first stage
 * creates the plug-ins, and the second stage invokes {@link Plugin#initialize} on
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
}

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
     * <code>Ice.InitPlugins=0</code> and then invoke {@link #initializePlugins}
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
    ["cpp:noexcept", "swift:noexcept"] StringSeq getPlugins();

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
    ["cpp:noexcept", "swift:noexcept"] void destroy();
}

#endif

}
