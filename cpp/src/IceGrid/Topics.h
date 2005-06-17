// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    NodeObserverTopic(const IceStorm::TopicPrx&, const NodeObserverPrx&);

    virtual void init(const NodeDynamicInfoSeq&, const Ice::Current&);
    virtual void initNode(const NodeDynamicInfo&, const Ice::Current&);
    virtual void updateServer(const std::string&, const ServerDynamicInfo&, const Ice::Current&);
    virtual void updateAdapter(const std::string&, const AdapterDynamicInfo&, const Ice::Current&);

    void subscribe(const NodeObserverPrx&, int serial = -1);
    void unsubscribe(const NodeObserverPrx&);

    void removeNode(const std::string&);

private:

    const IceStorm::TopicPrx _topic;
    const NodeObserverPrx _publisher;

    int _serial;
    std::map<std::string, NodeDynamicInfo> _nodes;
};
typedef IceUtil::Handle<NodeObserverTopic> NodeObserverTopicPtr;

class RegistryObserverTopic : public RegistryObserver, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RegistryObserverTopic(const IceStorm::TopicPrx&, const RegistryObserverPrx&, NodeObserverTopic&);
    virtual void init(int, const ApplicationDescriptorSeq&, const Ice::StringSeq&, const Ice::Current&);

    virtual void applicationAdded(int, const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void applicationRemoved(int, const std::string&, const Ice::Current&);
    virtual void applicationSynced(int, const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void applicationUpdated(int, const ApplicationUpdateDescriptor&, const Ice::Current&);

    virtual void nodeUp(const std::string&, const Ice::Current&);
    virtual void nodeDown(const std::string&, const Ice::Current&);

    void subscribe(const RegistryObserverPrx&, int = -1);
    void unsubscribe(const RegistryObserverPrx&);

private:

    void updateSerial(int);

    const IceStorm::TopicPrx _topic;
    const RegistryObserverPrx _publisher;
    NodeObserverTopic& _nodeObserver;

    int _serial;
    ApplicationDescriptorSeq _applications;
    Ice::StringSeq _nodes;
};
typedef IceUtil::Handle<RegistryObserverTopic> RegistryObserverTopicPtr;

};

#endif
