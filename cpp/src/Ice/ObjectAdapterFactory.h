// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

namespace IceInternal
{

class ObjectAdapterFactory : public ::IceUtil::Shared, public ::IceUtil::Monitor< ::IceUtil::RecMutex>
{
public:

    void shutdown();
    void waitForShutdown();
    bool isShutdown() const;
    void destroy();

    ::Ice::ObjectAdapterPtr createObjectAdapter(const std::string&, const std::string&, const Ice::RouterPrx&);
    ::Ice::ObjectAdapterPtr findObjectAdapter(const ::Ice::ObjectPrx&);
    void removeObjectAdapter(const ::std::string&);
    void flushBatchRequests() const;

private:

    ObjectAdapterFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);
    virtual ~ObjectAdapterFactory();
    friend class Instance;

    InstancePtr _instance;
    ::Ice::CommunicatorPtr _communicator;
    std::map<std::string, ::Ice::ObjectAdapterIPtr> _adapters;
    bool _waitForShutdown;
};

}

#endif
