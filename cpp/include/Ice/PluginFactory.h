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

    /// A plug-in factory function is a C function responsible for creating an Ice plug-in.
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

#ifndef ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT
#    define ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT ICE_DECLSPEC_IMPORT
#endif

    //
    // Checking for the API_EXPORTS macro is necessary to prevent inconsistent DLL linkage errors on Windows.
    //

#if !defined(ICE_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Returns the factory for the UDP transport plug-in, IceUDP.
    /// @return The factory for the IceUDP plug-in.
    /// @remark You only need to call this function if you are using static libraries. The Ice shared library adds the
    /// IceUDP plug-in automatically.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT PluginFactory udpPluginFactory();

    /// Returns the factory for the WebSocket transport plug-in, IceWS.
    /// @return The factory for the IceWS plug-in.
    /// @remark You only need to call this function if you are using static libraries. The Ice shared library adds the
    /// IceWS plug-in automatically.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT PluginFactory wsPluginFactory();
#endif

#if !defined(ICE_DISCOVERY_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Returns the factory for the IceDiscovery plug-in.
    /// @return The factory for the IceDiscovery plug-in.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT PluginFactory discoveryPluginFactory();
#endif

#if !defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Returns the factory for the IceLocatorDiscovery plug-in.
    /// @return The factory for the IceLocatorDiscovery plug-in.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT PluginFactory locatorDiscoveryPluginFactory();
#endif

#if (!defined(_WIN32) && !defined(__APPLE__)) || defined(ICE_DOXYGEN)
#    ifndef ICEBT_API_EXPORTS
    /// Returns the factory for the Bluetooth transport plug-in, IceBT.
    /// @return The factory for the IceBT plug-in.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT Pluginfactory btPluginFactory();
#    endif
#endif

#if (defined(__APPLE__) && TARGET_OS_IPHONE != 0) || defined(ICE_DOXYGEN)
#    ifndef ICEIAP_API_EXPORTS
    /// Returns the factory for the iAP transport plug-in, IceIAP.
    /// @return The factory for the IceIAP plug-in.
    /// @see InitializationData::plugins
    ICE_PLUGIN_FACTORY_DECLSPEC_IMPORT Pluginfactory iapPluginFactory();
#    endif
#endif

}

#endif
