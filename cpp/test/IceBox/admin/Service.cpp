// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceBox/IceBox.h"
#include "TestI.h"

using namespace std;
using namespace Ice;

class ServiceI : public ::IceBox::Service
{
public:
    ServiceI(const CommunicatorPtr&);
    ~ServiceI() override;

    void start(const string&, const CommunicatorPtr&, const StringSeq&) override;

    void stop() override;
};

extern "C"
{
    //
    // Factory function
    //
    ICE_DECLSPEC_EXPORT ::IceBox::Service* create(const CommunicatorPtr& communicator)
    {
        return new ServiceI(communicator);
    }
}

ServiceI::ServiceI(const CommunicatorPtr& serviceManagerCommunicator)
{
    TestFacetIPtr facet = std::make_shared<TestFacetI>();

    //
    // Install a custom admin facet.
    //
    serviceManagerCommunicator->addAdminFacet(facet, "TestFacet");

    //
    // Set the callback on the admin facet.
    //
    auto admin =
        serviceManagerCommunicator->findAdminFacet<NativePropertiesAdmin>("IceBox.Service.TestService.Properties");
    assert(admin);

    admin->addUpdateCallback([facet](const Ice::PropertyDict& changes) { facet->updated(changes); });
}

ServiceI::~ServiceI() = default;

void
ServiceI::start(const string&, const CommunicatorPtr&, const StringSeq&)
{
}

void
ServiceI::stop()
{
}
