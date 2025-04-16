// Copyright (c) ZeroC, Inc.

#ifndef ICE_PLUGIN_FACTORY_H
#define ICE_PLUGIN_FACTORY_H

#include "CommunicatorF.h"
#include "Config.h"
#include "Ice/BuiltinSequences.h"
#include "Plugin.h"

#include <string>

namespace Ice
{
    class Plugin;

    /// A plug-in factory function responsible for creating an Ice plug-in.
    /// For plug-ins that may be loaded dynamically, these functions are extern "C" to avoid name mangling.
    /// @param communicator The communicator in which the plug-in will be installed.
    /// @param name The name assigned to the plug-in.
    /// @param args Additional arguments included in the plug-in's configuration.
    /// @return The new plug-in object.
    using PluginFactoryFunc = Plugin* (*)(const CommunicatorPtr& communicator,
                                          const std::string& name,
                                          const StringSeq& args);

    /// Represents a plug-in factory.
    /// @see InitializeData::pluginFactories
    struct PluginFactory
    {
        /// The default and preferred name for plug-ins created by this factory.
        std::string pluginName;

        /// The factory function.
        PluginFactoryFunc factoryFunc;
    };

    // Transport plug-ins provided by the Ice library.

    /// Returns the factory for the UDP transport plug-in, IceUDP.
    /// @return The factory for the IceUDP plug-in.
    /// @remark You only need to call this function if you are using static libraries. The Ice shared library adds the
    /// IceUDP plug-in automatically.
    /// @see InitializationData::pluginFactories
    ICE_API PluginFactory udpPluginFactory();

    /// Returns the factory for the WebSocket transport plug-in, IceWS.
    /// @return The factory for the IceWS plug-in.
    /// @remark You only need to call this function if you are using static libraries. The Ice shared library adds the
    /// IceWS plug-in automatically.
    /// @see InitializationData::pluginFactories
    ICE_API PluginFactory wsPluginFactory();

#if (defined(__APPLE__) && TARGET_OS_IPHONE != 0) || defined(ICE_DOXYGEN)
    /// Returns the factory for the iAP transport plug-in, IceIAP.
    /// @return The factory for the IceIAP plug-in.
    /// @see InitializationData::pluginFactories
    ICE_API PluginFactory iapPluginFactory();
#endif

    /// @cond INTERNAL
    // Not exported because they are always registered by the Ice library.
    PluginFactory tcpPluginFactory();
    PluginFactory sslPluginFactory();
    /// @endcond
}

namespace IceInternal
{
    /// Indicates whether the current Ice library provides the minimum set of built-in plug-ins, namely IceTCP and
    /// IceSSL.
    /// @return `true` when the Ice library provides the minimum set of built-in plug-ins, `false` when it also provides
    /// IceUDP and IceWS.
    ICE_API bool isMinBuild();
}

#endif
