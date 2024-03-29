//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "IceBox/IceBox.h"
#include <TestI.h>

using namespace std;
using namespace Ice;

class ServiceI : public ::IceBox::Service
{
public:
    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&, const CommunicatorPtr&, const StringSeq&);

    virtual void stop();
};

extern "C"
{
    //
    // Factory function
    //
    ICE_DECLSPEC_EXPORT ::IceBox::Service* create(const CommunicatorPtr&) { return new ServiceI; }
}

ServiceI::ServiceI() {}

ServiceI::~ServiceI() {}

void
ServiceI::start(const string& name, const CommunicatorPtr& communicator, const StringSeq&)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name);
    adapter->add(make_shared<TestI>(properties), stringToIdentity(properties->getProperty(name + ".Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}
