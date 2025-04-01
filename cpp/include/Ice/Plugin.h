// Copyright (c) ZeroC, Inc.

#ifndef ICE_PLUGIN_H
#define ICE_PLUGIN_H

#include "Ice/BuiltinSequences.h"

#include <memory>

namespace Ice
{
    /// A communicator plug-in. A plug-in generally adds a feature to a communicator, such as support for a protocol.
    /// The communicator loads its plug-ins in two stages: the first stage creates the plug-ins, and the second stage
    /// invokes Plugin::initialize on each one.
    /// @headerfile Ice/Ice.h
    class ICE_API Plugin
    {
    public:
        virtual ~Plugin();

        /// Perform any necessary initialization steps.
        virtual void initialize() = 0;

        /// Called when the communicator is being destroyed.
        virtual void destroy() = 0;
    };

    /// A shared pointer to a Plugin.
    using PluginPtr = std::shared_ptr<Plugin>;

    /// Each communicator has a plug-in manager to administer the set of plug-ins.
    /// @headerfile Ice/Ice.h
    class ICE_API PluginManager
    {
    public:
        virtual ~PluginManager();

        /// Initialize the configured plug-ins. The communicator automatically initializes the plug-ins by default, but
        /// an application may need to interact directly with a plug-in prior to initialization. In this case, the
        /// application must set <code>Ice.InitPlugins=0</code> and then invoke `initializePlugins` manually. The
        /// plug-ins are initialized in the order in which they are loaded. If a plug-in throws an exception during
        /// initialization, the communicator invokes destroy on the plug-ins that have already been initialized.
        /// @throws InitializationException Thrown when the plug-ins have already been initialized.
        virtual void initializePlugins() = 0;

        /// Get a list of plugins installed.
        /// @return The names of the plugins installed.
        /// @see #getPlugin
        virtual StringSeq getPlugins() = 0;

        /// Obtain a plug-in by name.
        /// @param name The plug-in's name.
        /// @return The plug-in.
        /// @throws NotRegisteredException Thrown when no plug-in is found with the given name.
        virtual PluginPtr getPlugin(std::string_view name) = 0;

        /// Install a new plug-in.
        /// @param name The plug-in's name.
        /// @param pi The plug-in.
        /// @throws AlreadyRegisteredException Thrown when a plug-in already exists with the given name.
        virtual void addPlugin(std::string name, PluginPtr pi) = 0;

        /// Called when the communicator is being destroyed.
        virtual void destroy() noexcept = 0;
    };

    /// A shared pointer to a PluginManager.
    using PluginManagerPtr = std::shared_ptr<PluginManager>;
}

#endif
