// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <Freeze/Freeze.h>
#include <TestI.h>

#ifndef TEST_SERVICE_API
#   define TEST_SERVICE_API ICE_DECLSPEC_EXPORT
#endif


using namespace std;
using namespace Ice;

class TEST_SERVICE_API ServiceI : public ::IceBox::Service
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
		       const CommunicatorPtr&,
		       const StringSeq&);

    virtual void stop();
};

class TEST_SERVICE_API FreezeServiceI : public ::IceBox::FreezeService
{
public:

    FreezeServiceI();
    virtual ~FreezeServiceI();

    virtual void start(const string&,
		       const CommunicatorPtr&,
		       const StringSeq&,
		       const std::string&);

    virtual void stop();
};

extern "C"
{

//
// Factory function
//
TEST_SERVICE_API ::IceBox::Service*
create(CommunicatorPtr communicator)
{
    return new ServiceI;
}

TEST_SERVICE_API ::IceBox::FreezeService*
createFreezeService(CommunicatorPtr communicator)
{
    return new FreezeServiceI;
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
		const StringSeq& args)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name);
    Ice::ObjectPtr object = new TestI(adapter, properties);
    adapter->add(object, Ice::stringToIdentity(properties->getProperty(name + ".Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}

FreezeServiceI::FreezeServiceI()
{
}

FreezeServiceI::~FreezeServiceI()
{
}

void
FreezeServiceI::start(const string& name,
		      const CommunicatorPtr& communicator,
		      const StringSeq& args,
		      const std::string& envName)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name);
    Ice::ObjectPtr object = new TestI(adapter, communicator->getProperties());
    adapter->add(object, Ice::stringToIdentity(properties->getProperty(name + ".Identity")));
    adapter->activate();
}

void
FreezeServiceI::stop()
{
}
