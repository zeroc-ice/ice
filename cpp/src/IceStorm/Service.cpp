// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TraceLevels.h>
#include <IceBox/IceBox.h>

#if defined(_WIN32)
#   define ICESTORM_SERVICE_API __declspec(dllexport)
#else
#   define ICESTORM_SERVICE_API /**/
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Freeze;

namespace IceStorm
{

class ICESTORM_SERVICE_API ServiceI : public ::IceBox::FreezeService
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
                      const CommunicatorPtr&,
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
ICESTORM_SERVICE_API ::IceBox::FreezeService*
create(CommunicatorPtr communicator)
{
    return new ServiceI;
}

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
			  const StringSeq& args,
			  const DBEnvironmentPtr& dbEnv)
{
    DBPtr dbTopicManager = dbEnv->openDB("topicmanager", true);

    TraceLevelsPtr traceLevels = new TraceLevels(name, communicator->getProperties(), communicator->getLogger());
    _adapter = communicator->createObjectAdapter(name + ".TopicManager");

    _manager = new TopicManagerI(communicator, _adapter, traceLevels, dbEnv, dbTopicManager);
    _adapter->add(_manager, stringToIdentity(name + "/TopicManager"));

    _adapter->activate();
}

void
IceStorm::ServiceI::stop()
{
    _adapter->deactivate();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    _manager->reap();
}
