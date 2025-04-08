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

#if !defined(ICE_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Ensures the UDP transport plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property `Ice.Plugin.IceUDP` is
    /// set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceUDP(bool loadOnInitialize = true);

    /// Ensures the WebSocket transport plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property `Ice.Plugin.IceWS` is
    /// set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceWS(bool loadOnInitialize = true);
#endif

#if !defined(ICE_DISCOVERY_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Ensures the IceDiscovery plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property
    /// `Ice.Plugin.IceDiscovery` is set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceDiscovery(bool loadOnInitialize = true);
#endif

#if !defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS) || defined(ICE_DOXYGEN)
    /// Ensures the IceLocatorDiscovery plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property
    /// `Ice.Plugin.IceLocatorDiscovery` is set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceLocatorDiscovery(bool loadOnInitialize = true);
#endif

#if (!defined(_WIN32) && !defined(__APPLE__)) || defined(ICE_DOXYGEN)
#    ifndef ICEBT_API_EXPORTS
    /// Ensures the IceBT transport plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property `Ice.Plugin.IceBT` is
    /// set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceBT(bool loadOnInitialize = true);
#    endif
#endif

#if (defined(__APPLE__) && TARGET_OS_IPHONE != 0) || defined(ICE_DOXYGEN)
#    ifndef ICEIAP_API_EXPORTS
    /// Ensures the IceIAP transport plug-in is linked with the application.
    /// @param loadOnInitialize If `true`, the plug-in is always loaded (created) during communicator initialization.
    /// If `false`, the plug-in is only loaded during communicator initialization when property `Ice.Plugin.IceIAP` is
    /// set to 1.
    /// @remark You only need to call this function if you are using static libraries.
    ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceIAP(bool loadOnInitialize = true);
#    endif
#endif

}

#endif
