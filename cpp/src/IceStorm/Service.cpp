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

#ifndef ICESTORM_SERVICE_API
#   define ICESTORM_SERVICE_API ICE_DECLSPEC_EXPORT
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
		       const string&);

    virtual void stop();

private:

    TopicManagerIPtr _manager;
    ObjectAdapterPtr _topicAdapter;
    ObjectAdapterPtr _publishAdapter;
};

}

extern "C"
{

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
			  const string& envName)
{
    TraceLevelsPtr traceLevels = new TraceLevels(name, communicator->getProperties(), communicator->getLogger());
    _topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    _publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    _manager = new TopicManagerI(communicator, _topicAdapter, _publishAdapter, traceLevels, envName, "topics");
    _topicAdapter->add(_manager, stringToIdentity(name + "/TopicManager"));

    _topicAdapter->activate();
    _publishAdapter->activate();
}

void
IceStorm::ServiceI::stop()
{
    _topicAdapter->deactivate();
    _publishAdapter->deactivate();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    _manager->shutdown();
}
