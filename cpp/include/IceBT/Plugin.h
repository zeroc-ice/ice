// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_PLUGIN_H
#define ICE_BT_PLUGIN_H

#include <Ice/Plugin.h>

#ifndef ICE_BT_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_BT_API /**/
#   elif defined(ICE_BT_API_EXPORTS)
#       define ICE_BT_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_BT_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceBT
{

typedef std::map<std::string, std::string> PropertyMap;

#ifndef ICE_CPP11_MAPPING
//
// An application can receive discovery notifications
// by implementing the DiscoveryCallback interface.
//
class ICE_BT_API DiscoveryCallback : public IceUtil::Shared
{
public:

    //
    // Called for each discovered device. The same device may be reported multiple times.
    // The device's Bluetooth address is provided, along with a map of properties
    // supplied by the system's Bluetooth stack.
    //
    virtual void discovered(const std::string& addr, const PropertyMap& props) = 0;
};
typedef IceUtil::Handle<DiscoveryCallback> DiscoveryCallbackPtr;
#endif

class ICE_BT_API Plugin : public Ice::Plugin
{
public:

    //
    // Start Bluetooth device discovery on the adapter with the specified address.
    // The given callback will be invoked for each discovered device. The same
    // device may be reported more than once. Discovery remains active until
    // explicitly stopped by a call to stopDiscovery(), or via other administrative means.
    // The address argument can be an empty string to select the default adapter.
    //
#ifdef ICE_CPP11_MAPPING
    virtual void startDiscovery(const std::string& address,
                                std::function<void(const std::string& addr, const PropertyMap& props)>) = 0;
#else
    virtual void startDiscovery(const std::string& address, const DiscoveryCallbackPtr& cb) = 0;
#endif
    //
    // Stops Bluetooth device discovery on the adapter with the specified address.
    // The address argument can be an empty string to select the default adapter.
    // All discovery callbacks are removed when discovery stops.
    //
    virtual void stopDiscovery(const std::string& address) = 0;
};

}

#endif
