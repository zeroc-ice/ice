// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class FindServiceCallback : public IceUtil::Shared
{
public:

    virtual void completed(const std::vector<int>&) = 0;
    virtual void exception(const Ice::LocalException&) = 0;
};
typedef IceUtil::Handle<FindServiceCallback> FindServiceCallbackPtr;

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

    //
    // Blocks while we register a service with the Bluetooth daemon. Returns a
    // handle to the service that can be passed to removeService().
    //
    unsigned int addService(const std::string&, const std::string&, const std::string&, int);

    //
    // Asynchronously looks for a service at the given target device with the given UUID.
    //
    void findService(const std::string&, const std::string&, const FindServiceCallbackPtr&);

    //
    // Removes a service added by addService().
    //
    void removeService(const std::string&, unsigned int);

#ifdef ICE_CPP11_MAPPING
    void startDiscovery(const std::string&, std::function<void(const std::string&, const PropertyMap&)>);
#else
    void startDiscovery(const std::string&, const DiscoveryCallbackPtr&);
#endif
    void stopDiscovery(const std::string&);

    void destroy();

private:

    const Ice::CommunicatorPtr _communicator;
    bool _initialized;
    IceUtil::Monitor<IceUtil::Mutex> _lock;
    BluetoothServicePtr _service;
};

}

#endif
