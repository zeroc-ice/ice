// Copyright (c) ZeroC, Inc.

#ifndef ICE_REGISTER_PLUGINS_H
#define ICE_REGISTER_PLUGINS_H

#include "Config.h"

//
// Register functions for Ice plugins are declared here.
//
// These functions can be used to explicitly link with a plugin rather than relying on the loading of the plugin at
// runtime. The application must call the register function before initializing the communicator.
//

#ifndef ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT
#    define ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT ICE_DECLSPEC_IMPORT
#endif

namespace Ice
{
    //
    // Checking for the API_EXPORTS macro is necessary to prevent inconsistent DLL linkage errors on Windows.
    //

#ifndef ICE_API_EXPORTS
    /// When using static libraries, calling this function ensures the UDP transport is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceUDP(bool loadOnInitialize = true);

    /// When using static libraries, calling this function ensures the WebSocket transport is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceWS(bool loadOnInitialize = true);
#endif

#ifndef ICE_DISCOVERY_API_EXPORTS
    /// When using static libraries, calling this function ensures the IceDiscovery plug-in is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceDiscovery(bool loadOnInitialize = true);
#endif

#ifndef ICE_LOCATOR_DISCOVERY_API_EXPORTS
    /// When using static libraries, calling this function ensures the IceLocatorDiscovery plug-in is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceLocatorDiscovery(bool loadOnInitialize = true);
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
#    ifndef ICEBT_API_EXPORTS
    /// When using static libraries, calling this function ensures the IceBT plug-in is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceBT(bool loadOnInitialize = true);
#    endif
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE != 0
#    ifndef ICEIAP_API_EXPORTS
    /// When using static libraries, calling this function ensures the iAP plug-in is
    /// linked with the application.
    /// @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
    /// If false, the plug-in is only loaded during communicator initialization if its corresponding
    /// plug-in property is set to 1.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceIAP(bool loadOnInitialize = true);
#    endif
#endif

}

#endif
