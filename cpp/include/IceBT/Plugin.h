// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_PLUGIN_H
#define ICE_BT_PLUGIN_H

#include "Ice/Plugin.h"
#include "Types.h"

#include <functional>
#include <map>

namespace IceBT
{
    /// A name-value map.
    using PropertyMap = std::map<std::string, std::string>;

    /// A collection of properties for each device.
    using DeviceMap = std::map<std::string, PropertyMap>;

    /// Represents the IceBT plug-in object.
    /// @headerfile IceBT/IceBT.h
    class ICEBT_API Plugin : public Ice::Plugin
    {
    public:
        /// Start Bluetooth device discovery on the adapter with the specified address.
        /// The given callback will be invoked for each discovered device. The same
        /// device may be reported more than once. Discovery remains active until
        /// explicitly stopped by a call to stopDiscovery(), or via other administrative means.
        /// @param address The address associated with the Bluetooth adapter.
        /// @param cb The callback to invoke when a device is discovered.
        virtual void startDiscovery(
            const std::string& address,
            std::function<void(const std::string& addr, const PropertyMap& props)> cb) = 0;

        /// Stops Bluetooth device discovery on the adapter with the specified address.
        /// All discovery callbacks are removed when discovery stops.
        /// @param address The address associated with the Bluetooth adapter.
        virtual void stopDiscovery(const std::string& address) = 0;

        /// Retrieve a snapshot of all known remote devices. The plug-in obtains a snapshot of the remote devices at
        /// startup and then dynamically updates its map as the host adds and removes devices.
        /// @return A map containing properties for each known device.
        [[nodiscard]] virtual DeviceMap getDevices() const = 0;
    };
    using PluginPtr = std::shared_ptr<Plugin>;
}

#endif
