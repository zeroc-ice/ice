// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

class ServiceI : public ::IceBox::Service
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
                       const CommunicatorPtr&,
                       const StringSeq&);

    virtual void stop();
};

extern "C"
{

//
// Factory function
//
ICE_DECLSPEC_EXPORT ::IceBox::Service*
create(CommunicatorPtr)
{
    return new ServiceI;
}

}

ServiceI::ServiceI()
{
}

ServiceI::~ServiceI()
{
}

void
ServiceI::start(const string& name,
                const CommunicatorPtr& communicator,
                const StringSeq&)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name);
    Ice::ObjectPtr object = new TestI(communicator->getProperties());
    adapter->add(object, stringToIdentity(name));
    adapter->add(object, stringToIdentity(communicator->getProperties()->getProperty("Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}
