// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_TOPICS_H
#define ICEGRID_TOPICS_H

#include <IceUtil/Mutex.h>

#include <IceStorm/IceStorm.h>

#include <IceGrid/Observer.h>

namespace IceGrid
{

class NodeObserverTopic : public NodeObserver, public IceUtil::Mutex
{
public:

    NodeObserverTopic(const Ice::ObjectAdapterPtr&, const IceStorm::TopicManagerPrx&);

    virtual void init(const NodeDynamicInfoSeq&, const Ice::Current&);
    virtual void nodeUp(const NodeDynamicInfo&, const Ice::Current&);
    virtual void nodeDown(const std::string&, const Ice::Current&);
    virtual void updateServer(const std::string&, const ServerDynamicInfo&, const Ice::Current&);
    virtual void updateAdapter(const std::string&, const AdapterDynamicInfo&, const Ice::Current&);

    void subscribe(const NodeObserverPrx&, int serial = -1);
    void unsubscribe(const NodeObserverPrx&);
    const NodeObserverPrx& getPublisher() { return _publisher; }

    void removeNode(const std::string&);

private:

    const IceStorm::TopicPrx _topic;
    const NodeObserverPrx _internalPublisher;
    const NodeObserverPrx _publisher;

    int _serial;
    std::map<std::string, NodeDynamicInfo> _nodes;
};
typedef IceUtil::Handle<NodeObserverTopic> NodeObserverTopicPtr;

class RegistryObserverTopic : public RegistryObserver, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RegistryObserverTopic(const Ice::ObjectAdapterPtr&, const IceStorm::TopicManagerPrx&);
    virtual void init(int, const ApplicationDescriptorSeq&, const AdapterInfoSeq&, const ObjectInfoSeq&,
		      const Ice::Current&);

    virtual void applicationAdded(int, const ApplicationDescriptor&, const Ice::Current&);
    virtual void applicationRemoved(int, const std::string&, const Ice::Current&);
    virtual void applicationUpdated(int, const ApplicationUpdateDescriptor&, const Ice::Current&);

    virtual void adapterAdded(int, const AdapterInfo&, const Ice::Current&);
    virtual void adapterUpdated(int, const AdapterInfo&, const Ice::Current&);
    virtual void adapterRemoved(int, const std::string&, const Ice::Current&);
    
    virtual void objectAdded(int, const ObjectInfo&, const Ice::Current&);
    virtual void objectUpdated(int, const ObjectInfo&, const Ice::Current&);
    virtual void objectRemoved(int, const Ice::Identity&, const Ice::Current&);

    void subscribe(const RegistryObserverPrx&, int = -1);
    void unsubscribe(const RegistryObserverPrx&);
    const RegistryObserverPrx& getPublisher() { return _publisher; }

private:

    void updateSerial(int);

    const IceStorm::TopicPrx _topic;
    const RegistryObserverPrx _internalPublisher;
    const RegistryObserverPrx _publisher;

    int _serial;
    std::map<std::string, ApplicationDescriptor> _applications;
    std::map<std::string, AdapterInfo> _adapters;
    std::map<Ice::Identity, ObjectInfo> _objects;
};
typedef IceUtil::Handle<RegistryObserverTopic> RegistryObserverTopicPtr;

};

#endif
