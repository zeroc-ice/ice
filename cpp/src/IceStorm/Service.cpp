// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class ICESTORM_SERVICE_API ServiceI : public ::IceBox::Service
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
		       const CommunicatorPtr&,
		       const StringSeq&);

    virtual void stop();

private:

    TopicManagerIPtr _manager;
    ObjectAdapterPtr _topicAdapter;
    ObjectAdapterPtr _publishAdapter;
};

}

extern "C"
{

ICESTORM_SERVICE_API ::IceBox::Service*
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
			  const StringSeq& args)
{
    TraceLevelsPtr traceLevels = new TraceLevels(name, communicator->getProperties(), communicator->getLogger());
    _topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    _publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    //
    // We use the name of the service for the name of the database environment.
    //
    _manager = new TopicManagerI(communicator, _topicAdapter, _publishAdapter, traceLevels, name, "topics");
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
