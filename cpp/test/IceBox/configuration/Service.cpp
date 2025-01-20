// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceBox/IceBox.h"
#include "TestI.h"

using namespace std;
using namespace Ice;

class ServiceI : public ::IceBox::Service
{
public:
    ServiceI();
    ~ServiceI() override;

    void start(const string&, const CommunicatorPtr&, const StringSeq&) override;

    void stop() override;
};

extern "C"
{
    //
    // Factory function
    //
    ICE_DECLSPEC_EXPORT ::IceBox::Service* create(const CommunicatorPtr&) { return new ServiceI; }
}

ServiceI::ServiceI() = default;

ServiceI::~ServiceI() = default;

void
ServiceI::start(const string& name, const CommunicatorPtr& communicator, const StringSeq& args)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name + "OA");
    adapter->add(make_shared<TestI>(args), stringToIdentity("test"));
    adapter->activate();
}

void
ServiceI::stop()
{
}
