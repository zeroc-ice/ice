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

namespace IceStorm
{

class Service : public ::IceBox::FreezeService
{
public:

    Service();
    virtual ~Service();

    virtual void start(const ::std::string&,
		       const ::Ice::CommunicatorPtr&,
		       const ::Ice::PropertiesPtr&,
		       const ::Ice::StringSeq&,
		       const ::Freeze::DBEnvironmentPtr&);

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
create(Ice::CommunicatorPtr communicator)
{
    return new ::IceStorm::Service;
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
			 const ::Ice::CommunicatorPtr& communicator,
			 const ::Ice::PropertiesPtr& properties,
			 const ::Ice::StringSeq& args,
			 const ::Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr dbTopicManager = dbEnv->openDB("topicmanager", true);

    TraceLevelsPtr traceLevels = new TraceLevels(communicator->getProperties(), communicator->getLogger());
    _adapter = communicator->createObjectAdapterFromProperty("TopicManager", "IceStorm.TopicManager.Endpoints");
    _manager = new TopicManagerI(communicator, _adapter, traceLevels, dbEnv, dbTopicManager);
    _adapter->add(_manager, stringToIdentity("TopicManager"));

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
