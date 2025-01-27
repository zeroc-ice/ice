// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_ENGINE_H
#define ICE_BT_ENGINE_H

#include "Config.h"
#include "EngineF.h"
#include "Ice/CommunicatorF.h"
#include "IceBT/Plugin.h"
#include "IceBT/Types.h"

#include <mutex>

namespace IceBT
{
    //
    // Notifies the transport about a new incoming connection.
    //
    class ProfileCallback
    {
    public:
        virtual void newConnection(int) = 0;
    };
    using ProfileCallbackPtr = std::shared_ptr<ProfileCallback>;

    //
    // Represents an outgoing (client) connection. The transport must keep a reference to this object
    // and call close() when no longer needed.
    //
    class Connection
    {
    public:
        virtual void close() = 0;
    };
    using ConnectionPtr = std::shared_ptr<Connection>;

    //
    // Callback API for an outgoing connection attempt.
    //
    class ConnectCallback
    {
    public:
        virtual void completed(int, const ConnectionPtr&) = 0;
        virtual void failed(std::exception_ptr) = 0;
    };
    using ConnectCallbackPtr = std::shared_ptr<ConnectCallback>;

    //
    // Engine encapsulates all Bluetooth activities.
    //
    class Engine
    {
    public:
        Engine(Ice::CommunicatorPtr);

        Ice::CommunicatorPtr communicator() const;

        void initialize();
        bool initialized() const;

        std::string getDefaultAdapterAddress() const;
        bool adapterExists(const std::string&) const;

        bool deviceExists(const std::string&) const;

        std::string registerProfile(const std::string&, const std::string&, int, const ProfileCallbackPtr&);
        void unregisterProfile(const std::string&);

        void connect(const std::string&, const std::string&, const ConnectCallbackPtr&);

        void startDiscovery(const std::string&, std::function<void(const std::string&, const PropertyMap&)>);
        void stopDiscovery(const std::string&);

        DeviceMap getDevices() const;

        void destroy();

    private:
        const Ice::CommunicatorPtr _communicator;
        bool _initialized{false};
        mutable std::mutex _mutex;
        BluetoothServicePtr _service;
    };
}

#endif
