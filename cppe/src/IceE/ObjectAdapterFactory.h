// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_FACTORY_H
#define ICEE_OBJECT_ADAPTER_FACTORY_H

#include <IceE/ObjectAdapterF.h>
#ifdef ICEE_HAS_ROUTER
# include <IceE/RouterF.h>
#endif
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
    bool isShutdown() const;
    void destroy();

    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&, const std::string&
#ifdef ICEE_HAS_ROUTER
						, const ::Ice::RouterPrx&
#endif
    					        );
    void removeObjectAdapter(const std::string&);
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
