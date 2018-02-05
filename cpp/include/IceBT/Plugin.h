// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_PLUGIN_H
#define ICE_BT_PLUGIN_H

#include <Ice/Plugin.h>

#ifndef ICEBT_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEBT_API /**/
#   elif defined(ICEBT_API_EXPORTS)
#       define ICEBT_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEBT_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceBT
{

/** A name-value map. */
typedef std::map<std::string, std::string> PropertyMap;

/** A collection of properties for each device. */
typedef std::map<std::string, PropertyMap> DeviceMap;

#ifndef ICE_CPP11_MAPPING
/**
 * An application can receive discovery notifications by implementing the DiscoveryCallback interface.
 * \headerfile IceBT/IceBT.h
 */
class ICEBT_API DiscoveryCallback : public IceUtil::Shared
{
public:

    /**
     * Called for each discovered device. The same device may be reported multiple times.
     * @param addr The discovered device's Bluetooth address.
     * @param props A map of device properties supplied by the system's Bluetooth stack.
     */
    virtual void discovered(const std::string& addr, const PropertyMap& props) = 0;
};
typedef IceUtil::Handle<DiscoveryCallback> DiscoveryCallbackPtr;
#endif

/**
 * Represents the IceBT plug-in object.
 * \headerfile IceBT/IceBT.h
 */
class ICEBT_API Plugin : public Ice::Plugin
{
public:

    /**
     * Start Bluetooth device discovery on the adapter with the specified address.
     * The given callback will be invoked for each discovered device. The same
     * device may be reported more than once. Discovery remains active until
     * explicitly stopped by a call to stopDiscovery(), or via other administrative means.
     * @param address The address associated with the Bluetooth adapter.
     * @param cb The callback to invoke when a device is discovered.
     */
#ifdef ICE_CPP11_MAPPING
    virtual void startDiscovery(const std::string& address,
                                std::function<void(const std::string& addr, const PropertyMap& props)> cb) = 0;
#else
    virtual void startDiscovery(const std::string& address, const DiscoveryCallbackPtr& cb) = 0;
#endif

    /**
     * Stops Bluetooth device discovery on the adapter with the specified address.
     * All discovery callbacks are removed when discovery stops.
     * @param address The address associated with the Bluetooth adapter.
     */
    virtual void stopDiscovery(const std::string& address) = 0;

    /**
     * Retrieve a snapshot of all known remote devices. The plug-in obtains a snapshot of the remote devices at
     * startup and then dynamically updates its map as the host adds and removes devices.
     * @return A map containing properties for each known device.
     */
    virtual DeviceMap getDevices() const = 0;
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

}

#endif
