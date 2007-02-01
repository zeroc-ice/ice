// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    if(properties->getPropertyAsInt(name + ".Freeze") > 0)
    {
        //
        // We do this to ensure the dbenv directory exists.
        //
        Freeze::createConnection(communicator, name);
    }
    Ice::ObjectPtr object = new TestI(adapter, properties);
    adapter->add(object, communicator->stringToIdentity(properties->getProperty(name + ".Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}

