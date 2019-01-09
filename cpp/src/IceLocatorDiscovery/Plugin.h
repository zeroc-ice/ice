// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef LOCATOR_DISCOVERY_PLUGIN_I_H
#define LOCATOR_DISCOVERY_PLUGIN_I_H

#include <Ice/Config.h>
#include <Ice/Plugin.h>
#include <Ice/Locator.h>
#include <IceUtil/Time.h>

//
// Automatically link IceLocatorDiscovery[D|++11|++11D].lib with Visual C++
//
#if !defined(ICE_BUILDING_ICE_LOCATOR_DISCOVERY) && defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS)
#   define ICE_BUILDING_ICE_LOCATOR_DISCOVERY
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_LOCATOR_DISCOVERY)
#   pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery"))
#endif

#ifndef ICE_LOCATOR_DISCOVERY_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_LOCATOR_DISCOVERY_API /**/
#   elif defined(ICE_LOCATOR_DISCOVERY_API_EXPORTS)
#       define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceLocatorDiscovery
{

class ICE_LOCATOR_DISCOVERY_API Plugin : public Ice::Plugin
{
public:

    virtual std::vector<Ice::LocatorPrxPtr> getLocators(const std::string&, const IceUtil::Time&) const = 0;
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

};

#endif
