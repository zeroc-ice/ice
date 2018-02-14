// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    ServiceI(const CommunicatorPtr&);
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
create(CommunicatorPtr communicator)
{
    return new ServiceI(communicator);
}

}

ServiceI::ServiceI(const CommunicatorPtr& serviceManagerCommunicator)
{
    TestFacetIPtr facet = new TestFacetI;

    //
    // Install a custom admin facet.
    //
    serviceManagerCommunicator->addAdminFacet(facet, "TestFacet");

    //
    // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
    // Set the callback on the admin facet.
    //
    ObjectPtr propFacet = serviceManagerCommunicator->findAdminFacet("IceBox.Service.TestService.Properties");
    NativePropertiesAdminPtr admin = NativePropertiesAdminPtr::dynamicCast(propFacet);
    assert(admin);
    admin->addUpdateCallback(facet);
}

ServiceI::~ServiceI()
{
}

void
ServiceI::start(const string&, const CommunicatorPtr&, const StringSeq&)
{
}

void
ServiceI::stop()
{
}
