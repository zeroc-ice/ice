// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
// thanrelying on the loading of the plugin at runtime. The application
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
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceStringConverter(bool = true);
#endif

#ifndef ICE_SSL_API_EXPORTS
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceSSL(bool = true);
#endif

#ifndef ICE_DISCOVERY_API_EXPORTS
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceDiscovery(bool = true);
#endif

#ifndef ICE_LOCATOR_DISCOVERY_API_EXPORTS
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceLocatorDiscovery(bool = true);
#endif

}

#endif
