// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TraceLevels.h>
#include <IceBox/IceBox.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Freeze;

namespace IceStorm
{

class Service : public ::IceBox::FreezeService
{
public:

    Service();
    virtual ~Service();

    virtual void start(const string&,
                      const CommunicatorPtr&,
                      const PropertiesPtr&,
                      const StringSeq&,
                      const DBEnvironmentPtr&);

    virtual void stop();

private:

    TopicManagerIPtr _manager;
    ObjectAdapterPtr _adapter;
};

} // End namespace IceStorm

extern "C"
{

//
// Factory function
//
::IceBox::FreezeService*
create(CommunicatorPtr communicator)
{
    return new Service;
}

}

IceStorm::Service::Service()
{
}

IceStorm::Service::~Service()
{
}

void
IceStorm::Service::start(const string& name,
			 const CommunicatorPtr& communicator,
			 const PropertiesPtr& properties,
			 const StringSeq& args,
			 const DBEnvironmentPtr& dbEnv)
{
    DBPtr dbTopicManager = dbEnv->openDB("topicmanager", true);

    TraceLevelsPtr traceLevels = new TraceLevels(name, properties, communicator->getLogger());
    string endpoints = properties->getProperty(name + ".TopicManager.Endpoints");
    _adapter = communicator->createObjectAdapterWithEndpoints(name + ".TopicManagerAdapter", endpoints);
    _manager = new TopicManagerI(communicator, _adapter, traceLevels, dbEnv, dbTopicManager);
    _adapter->add(_manager, stringToIdentity(name + ".TopicManager"));

    _adapter->activate();
}

void
IceStorm::Service::stop()
{
    _adapter->deactivate();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    _manager->reap();
}
