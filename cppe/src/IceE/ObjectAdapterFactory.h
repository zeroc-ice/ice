// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_FACTORY_H
#define ICEE_OBJECT_ADAPTER_FACTORY_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Monitor.h>

#include <IceE/ObjectAdapterF.h>
#include <IceE/ProxyF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/InstanceF.h>

namespace IceEInternal
{

class ObjectAdapterFactory : public ::IceE::Shared, public ::IceE::Monitor< ::IceE::Mutex>
{
public:

    void shutdown();
    void waitForShutdown();

    ::IceE::ObjectAdapterPtr createObjectAdapter(const std::string&);
    ::IceE::ObjectAdapterPtr findObjectAdapter(const ::IceE::ObjectPrx&);
    void flushBatchRequests() const;

private:

    ObjectAdapterFactory(const InstancePtr&, const ::IceE::CommunicatorPtr&);
    virtual ~ObjectAdapterFactory();
    friend class Instance;

    InstancePtr _instance;
    ::IceE::CommunicatorPtr _communicator;
    std::map<std::string, ::IceE::ObjectAdapterPtr> _adapters;
    bool _waitForShutdown;
};

}

#endif
