// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceBox/IceBox.h"
#include "TestI.h"

using namespace std;
using namespace Ice;

class ServiceI final : public IceBox::Service
{
public:
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

void
ServiceI::start(const string& name, const CommunicatorPtr& communicator, const StringSeq&)
{
    auto adapter = communicator->createObjectAdapter(name);
    auto object = make_shared<TestI>(communicator->getProperties());
    adapter->add(object, stringToIdentity(name));
    adapter->add(object, stringToIdentity(communicator->getProperties()->getProperty("Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}
