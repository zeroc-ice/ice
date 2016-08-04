// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_H
#define ICE_OBJECT_ADAPTER_FACTORY_H

#include <Ice/ObjectAdapterI.h>
#include <IceUtil/RecMutex.h>
#include <IceUtil/Monitor.h>

#include <set>

namespace IceInternal
{

class ObjectAdapterFactory : public ::IceUtil::Monitor< ::IceUtil::RecMutex>,
#ifdef ICE_CPP11_MAPPING
                             public std::enable_shared_from_this<ObjectAdapterFactory>
#else
                             public virtual IceUtil::Shared
#endif
{
public:

    void shutdown();
    void waitForShutdown();
    bool isShutdown() const;
    void destroy();

    void updateObservers(void (Ice::ObjectAdapterI::*)());

    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&, const Ice::RouterPrxPtr&);
    ::Ice::ObjectAdapterPtr findObjectAdapter(const ::Ice::ObjectPrxPtr&);
    void removeObjectAdapter(const ::Ice::ObjectAdapterPtr&);
    void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&) const;

    ObjectAdapterFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);
    virtual ~ObjectAdapterFactory();

private:

    friend class Instance;

    InstancePtr _instance;
    ::Ice::CommunicatorPtr _communicator;
    std::set<std::string> _adapterNamesInUse;
    std::list<Ice::ObjectAdapterIPtr> _adapters;
};

}

#endif
