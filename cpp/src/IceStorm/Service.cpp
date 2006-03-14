// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TraceLevels.h>
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
    ObjectAdapterPtr _topicAdapter;
    ObjectAdapterPtr _publishAdapter;
};

}

extern "C"
{

ICE_STORM_SERVICE_API ::IceBox::Service*
create(CommunicatorPtr communicator)
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
IceStorm::ServiceI::start(const string& name,
			  const CommunicatorPtr& communicator,
			  const StringSeq& args)
{
    PropertiesPtr properties = communicator->getProperties();
    TraceLevelsPtr traceLevels = new TraceLevels(name, properties, communicator->getLogger());
    _topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    _publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    //
    // We use the name of the service for the name of the database environment.
    //
    string topicManagerId = properties->getPropertyWithDefault(name + ".InstanceName", "IceStorm") + "/TopicManager";
    Ice::Identity id = stringToIdentity(topicManagerId);
    _manager = new TopicManagerI(communicator, _topicAdapter, _publishAdapter, traceLevels, name, "topics");
    _managerProxy = TopicManagerPrx::uncheckedCast(_topicAdapter->add(_manager, id));

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
    TraceLevelsPtr traceLevels = new TraceLevels(name, communicator->getProperties(), communicator->getLogger());

    //
    // We use the name of the service for the name of the database environment.
    //
    _manager = new TopicManagerI(communicator, topicAdapter, publishAdapter, traceLevels, dbEnv, "topics");
    _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->add(_manager, id));
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
	_topicAdapter->deactivate();
    }
    if(_publishAdapter)
    {
	_publishAdapter->deactivate();
    }

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    _manager->shutdown();
}
