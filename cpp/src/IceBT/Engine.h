// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_ENGINE_H
#define ICE_BT_ENGINE_H

#include <IceBT/EngineF.h>
#include <IceBT/Config.h>
#include <IceBT/Plugin.h>
#include <IceBT/Types.h>
#include <Ice/CommunicatorF.h>

namespace IceBT
{

//
// Notifies the transport about a new incoming connection.
//
class ProfileCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual void newConnection(int) = 0;
};
ICE_DEFINE_PTR(ProfileCallbackPtr, ProfileCallback);

//
// Represents an outgoing (client) connection. The transport must keep a reference to this object
// and call close() when no longer needed.
//
class Connection : public IceUtil::Shared
{
public:

    virtual void close() = 0;

};
typedef IceUtil::Handle<Connection> ConnectionPtr;

//
// Callback API for an outgoing connection attempt.
//
class ConnectCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual void completed(int, const ConnectionPtr&) = 0;
    virtual void failed(const Ice::LocalException&) = 0;
};
ICE_DEFINE_PTR(ConnectCallbackPtr, ConnectCallback);

//
// Engine encapsulates all Bluetooth activities.
//
class Engine : public IceUtil::Shared
{
public:

    Engine(const Ice::CommunicatorPtr&);

    Ice::CommunicatorPtr communicator() const;

    void initialize();
    bool initialized() const;

    std::string getDefaultAdapterAddress() const;
    bool adapterExists(const std::string&) const;

    bool deviceExists(const std::string&) const;

    std::string registerProfile(const std::string&, const std::string&, int, const ProfileCallbackPtr&);
    void unregisterProfile(const std::string&);

    void connect(const std::string&, const std::string&, const ConnectCallbackPtr&);

#ifdef ICE_CPP11_MAPPING
    void startDiscovery(const std::string&, std::function<void(const std::string&, const PropertyMap&)>);
#else
    void startDiscovery(const std::string&, const DiscoveryCallbackPtr&);
#endif
    void stopDiscovery(const std::string&);

    DeviceMap getDevices() const;

    void destroy();

private:

    const Ice::CommunicatorPtr _communicator;
    bool _initialized;
    IceUtil::Monitor<IceUtil::Mutex> _lock;
    BluetoothServicePtr _service;
};

}

#endif
