// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/BatchFlusher.h>
#include <IceStorm/SubscriberPool.h>
#include <IceStorm/Service.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Freeze;

namespace IceStorm
{

class ServiceI : public ::IceStorm::Service
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
                       const CommunicatorPtr&,
                       const StringSeq&);

    virtual void start(const CommunicatorPtr&, 
                       const ObjectAdapterPtr&, 
                       const ObjectAdapterPtr&,
                       const string&, 
                       const Ice::Identity&,
                       const string&);

    virtual TopicManagerPrx getTopicManager() const;    

    virtual void stop();

private:

    TopicManagerIPtr _manager;
    TopicManagerPrx _managerProxy;
    InstancePtr _instance;
    ObjectAdapterPtr _topicAdapter;
    ObjectAdapterPtr _publishAdapter;
};

}

extern "C"
{

ICE_STORM_API ::IceBox::Service*
createIceStorm(CommunicatorPtr communicator)
{
    return new ServiceI;
}

}

ServicePtr
IceStorm::Service::create(const CommunicatorPtr& communicator,
                          const ObjectAdapterPtr& topicAdapter,
                          const ObjectAdapterPtr& publishAdapter,
                          const string& name,
                          const Ice::Identity& id,
                          const string& dbEnv)
{
    ServiceI* service = new ServiceI;
    ServicePtr svc = service;
    service->start(communicator, topicAdapter, publishAdapter, name, id, dbEnv);
    return svc;
}

IceStorm::ServiceI::ServiceI()
{
}

IceStorm::ServiceI::~ServiceI()
{
}

void
IceStorm::ServiceI::start(
    const string& name,
    const CommunicatorPtr& communicator,
    const StringSeq& args)
{
    PropertiesPtr properties = communicator->getProperties();

    _topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    _publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    //
    // We use the name of the service for the name of the database environment.
    //
    string instanceName = properties->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    Identity topicManagerId;
    topicManagerId.category = instanceName;
    topicManagerId.name = "TopicManager";

    _instance = new Instance(instanceName, name, communicator, _publishAdapter);
    
    try
    {
        _manager = new TopicManagerI(_instance, _topicAdapter, name, "topics");
        _managerProxy = TopicManagerPrx::uncheckedCast(_topicAdapter->add(_manager, topicManagerId));
    }
    catch(const Ice::Exception&)
    {
        _instance = 0;
        throw;
    }
        
    _topicAdapter->activate();
    _publishAdapter->activate();
}

void
IceStorm::ServiceI::start(const CommunicatorPtr& communicator,
                          const ObjectAdapterPtr& topicAdapter,
                          const ObjectAdapterPtr& publishAdapter,
                          const string& name,
                          const Ice::Identity& id,
                          const string& dbEnv)
{
    string instanceName = communicator->getProperties()->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    _instance = new Instance(instanceName, name, communicator, publishAdapter);

    //
    // We use the name of the service for the name of the database environment.
    //
    try
    {
        _manager = new TopicManagerI(_instance, topicAdapter, dbEnv, "topics");
        _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->add(_manager, id));
    }
    catch(const Ice::Exception&)
    {
        _instance = 0;
        throw;
    }
}

TopicManagerPrx
IceStorm::ServiceI::getTopicManager() const
{
    return _managerProxy;
}

void
IceStorm::ServiceI::stop()
{
    if(_topicAdapter)
    {
        _topicAdapter->destroy();
    }
    if(_publishAdapter)
    {
        _publishAdapter->destroy();
    }

    //
    // Shutdown the instance.
    //
    _instance->shutdown();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    _manager->shutdown();
}
