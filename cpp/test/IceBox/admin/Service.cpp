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
#ifdef ICE_CPP11_MAPPING
create(const shared_ptr<Communicator>& communicator)
#else
create(CommunicatorPtr communicator)
#endif
{
    return new ServiceI(communicator);
}

}

ServiceI::ServiceI(const CommunicatorPtr& serviceManagerCommunicator)
{
    TestFacetIPtr facet = ICE_MAKE_SHARED(TestFacetI);

    //
    // Install a custom admin facet.
    //
    serviceManagerCommunicator->addAdminFacet(facet, "TestFacet");

    //
    // The TestFacetI servant also implements PropertiesAdminUpdateCallback.
    // Set the callback on the admin facet.
    //
    ObjectPtr propFacet = serviceManagerCommunicator->findAdminFacet("IceBox.Service.TestService.Properties");
    NativePropertiesAdminPtr admin = ICE_DYNAMIC_CAST(NativePropertiesAdmin, propFacet);
    assert(admin);

#ifdef ICE_CPP11_MAPPING
    admin->addUpdateCallback([facet](const Ice::PropertyDict& changes) { facet->updated(changes); });
#else
    admin->addUpdateCallback(facet);
#endif
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
