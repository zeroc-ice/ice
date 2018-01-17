// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REGISTER_PLUGINS_H
#define ICE_REGISTER_PLUGINS_H

#include <Ice/Config.h>

//
// Register functions for Ice plugins are declared here.
//
// These functions can be used to explicitly link with a plugin rather
// than relying on the loading of the plugin at runtime. The application
// must call the register function before initializing the communicator.
//

namespace Ice
{

#if defined(ICE_STATIC_LIBS)
#  define ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT /**/
#else
#  define ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT ICE_DECLSPEC_IMPORT
#endif

//
// Checking for the API_EXPORTS macro is necessary to prevent
// inconsistent DLL linkage errors on Windows.
//

#ifndef ICE_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the string converter plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceStringConverter(bool loadOnInitialize = true);

/**
 * When using static libraries, calling this function ensures the UDP transport is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceUDP(bool loadOnInitialize = true);

/**
 * When using static libraries, calling this function ensures the WebSocket transport is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceWS(bool loadOnInitialize = true);
#endif

#ifndef ICESSL_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the SSL transport is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceSSL(bool loadOnInitialize = true);
#endif

#ifndef ICE_DISCOVERY_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the IceDiscovery plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceDiscovery(bool loadOnInitialize = true);
#endif

#ifndef ICE_LOCATOR_DISCOVERY_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the IceLocatorDiscovery plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceLocatorDiscovery(bool loadOnInitialize = true);
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
#   ifndef ICEBT_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the IceBT plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceBT(bool loadOnInitialize = true);
#   endif
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE != 0
#   ifndef ICEIAP_API_EXPORTS
/**
 * When using static libraries, calling this function ensures the iAP plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding
 * plug-in property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceIAP(bool loadOnInitialize = true);
#   endif
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_SRC)
#   pragma comment(lib, ICE_LIBNAME("IceDiscovery"))
#   pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery"))
#   pragma comment(lib, ICE_LIBNAME("IceSSL"))
#endif
}

#endif
