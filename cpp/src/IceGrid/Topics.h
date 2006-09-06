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
#include <IceGrid/Internal.h>
#include <IceGrid/Observer.h>
#include <IceGrid/StringApplicationInfoDict.h>
#include <IceGrid/StringAdapterInfoDict.h>
#include <IceGrid/IdentityObjectInfoDict.h>

namespace IceGrid
{
enum TopicName
{
    RegistryObserverTopicName,
    NodeObserverTopicName,
    ApplicationObserverTopicName,
    AdapterObserverTopicName,
    ObjectObserverTopicName
};    

class ObserverTopic : public IceUtil::Monitor<IceUtil::Mutex>, virtual public Ice::Object
{
public:

    ObserverTopic(const IceStorm::TopicManagerPrx&, const std::string&);
    virtual ~ObserverTopic();

    void subscribe(const Ice::ObjectPrx&, int = -1);
    void subscribeAndWaitForSubscription(const Ice::ObjectPrx&);
    void unsubscribe(const Ice::ObjectPrx&);
    void destroy();

    virtual void initObserver(const Ice::ObjectPrx&) = 0;

protected:

    void subscribeImpl(const Ice::ObjectPrx&);
    void updateSerial(int);
    Ice::Context getContext(const std::string&, int) const;

    IceStorm::TopicPrx _topic;
    Ice::ObjectPrx _basePublisher;
    std::set<Ice::Identity> _waitForSubscribe;
    int _serial;    
};
typedef IceUtil::Handle<ObserverTopic> ObserverTopicPtr;

class RegistryObserverTopic : public ObserverTopic
{
public:

    RegistryObserverTopic(const IceStorm::TopicManagerPrx&);

    void registryUp(const RegistryInfo&);
    void registryDown(const std::string&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    const RegistryObserverPrx _publisher;
    std::map<std::string, RegistryInfo> _registries;
};
typedef IceUtil::Handle<RegistryObserverTopic> RegistryObserverTopicPtr;

class NodeObserverTopic : public ObserverTopic, public NodeObserver
{
public:
    
    NodeObserverTopic(const IceStorm::TopicManagerPrx&, const Ice::ObjectAdapterPtr&);

    virtual void nodeInit(const NodeDynamicInfoSeq&, const Ice::Current&);
    virtual void nodeUp(const NodeDynamicInfo&, const Ice::Current&);
    virtual void nodeDown(const std::string&, const Ice::Current&);
    virtual void updateServer(const std::string&, const ServerDynamicInfo&, const Ice::Current&);
    virtual void updateAdapter(const std::string&, const AdapterDynamicInfo&, const Ice::Current&);

    const NodeObserverPrx& getPublisher() { return _externalPublisher; }

    void nodeDown(const std::string&);
    virtual void initObserver(const Ice::ObjectPrx&);

private:

    const NodeObserverPrx _externalPublisher;
    const NodeObserverPrx _publisher;
    std::map<std::string, NodeDynamicInfo> _nodes;
};
typedef IceUtil::Handle<NodeObserverTopic> NodeObserverTopicPtr;

class ApplicationObserverTopic : public ObserverTopic
{
public:

    ApplicationObserverTopic(const IceStorm::TopicManagerPrx&, const StringApplicationInfoDict&);

    void applicationInit(int, const ApplicationInfoSeq&);
    void applicationAdded(int, const ApplicationInfo&);
    void applicationRemoved(int, const std::string&);
    void applicationUpdated(int, const ApplicationUpdateInfo&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    const ApplicationObserverPrx _publisher;
    std::map<std::string, ApplicationInfo> _applications;
};
typedef IceUtil::Handle<ApplicationObserverTopic> ApplicationObserverTopicPtr;

class AdapterObserverTopic : public ObserverTopic
{
public:

    AdapterObserverTopic(const IceStorm::TopicManagerPrx&, const StringAdapterInfoDict&);

    void adapterInit(int, const AdapterInfoSeq&);
    void adapterAdded(int, const AdapterInfo&);
    void adapterUpdated(int, const AdapterInfo&);
    void adapterRemoved(int, const std::string&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    const AdapterObserverPrx _publisher;
    std::map<std::string, AdapterInfo> _adapters;
};
typedef IceUtil::Handle<AdapterObserverTopic> AdapterObserverTopicPtr;

class ObjectObserverTopic : public ObserverTopic
{
public:

    ObjectObserverTopic(const IceStorm::TopicManagerPrx&, const IdentityObjectInfoDict&);

    void objectInit(int, const ObjectInfoSeq&);
    void objectAdded(int, const ObjectInfo&);
    void objectUpdated(int, const ObjectInfo&);
    void objectRemoved(int, const Ice::Identity&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    const ObjectObserverPrx _publisher;
    std::map<Ice::Identity, ObjectInfo> _objects;
};
typedef IceUtil::Handle<ObjectObserverTopic> ObjectObserverTopicPtr;

};

#endif
