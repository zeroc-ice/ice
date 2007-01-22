// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTANCE_H
#define INSTANCE_H


#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <IceUtil/Time.h>

namespace IceStorm
{

class BatchFlusher;
typedef IceUtil::Handle<BatchFlusher> BatchFlusherPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class SubscriberPool;
typedef IceUtil::Handle<SubscriberPool> SubscriberPoolPtr;

class Instance : public IceUtil::Shared
{
public:

    Instance(const std::string&, const std::string&, const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&);
    ~Instance();

    std::string instanceName() const;
    Ice::CommunicatorPtr communicator() const;
    Ice::PropertiesPtr properties() const;
    Ice::ObjectAdapterPtr objectAdapter() const;
    TraceLevelsPtr traceLevels() const;
    BatchFlusherPtr batchFlusher() const;
    SubscriberPoolPtr subscriberPool() const;

    IceUtil::Time discardInterval() const;
    int sendTimeout() const;

    void shutdown();

private:

    const std::string _instanceName;
    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _adapter;
    const TraceLevelsPtr _traceLevels;
    const IceUtil::Time _discardInterval;
    const int _sendTimeout;
    BatchFlusherPtr _batchFlusher;
    SubscriberPoolPtr _subscriberPool;
};
typedef IceUtil::Handle<Instance> InstancePtr;

} // End namespace IceStorm

#endif
