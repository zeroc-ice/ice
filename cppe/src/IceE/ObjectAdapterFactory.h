// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_FACTORY_H
#define ICEE_OBJECT_ADAPTER_FACTORY_H

#include <IceE/ObjectAdapterF.h>
#include <IceE/ProxyF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/InstanceF.h>

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Monitor.h>

namespace IceInternal
{

class ObjectAdapterFactory : public ::IceUtil::Shared, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void shutdown();
    void waitForShutdown();

    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&, const std::string&);
    ::Ice::ObjectAdapterPtr findObjectAdapter(const ::Ice::ObjectPrx&);
    void flushBatchRequests() const;

private:

    ObjectAdapterFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);
    virtual ~ObjectAdapterFactory();
    friend class Instance;

    InstancePtr _instance;
    ::Ice::CommunicatorPtr _communicator;
    std::map<std::string, ::Ice::ObjectAdapterPtr> _adapters;
    bool _waitForShutdown;
};

}

#endif
