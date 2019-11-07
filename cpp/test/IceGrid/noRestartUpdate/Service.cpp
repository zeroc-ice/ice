//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

class ServiceI final : public IceBox::Service
{
public:

    void start(const string&,
               const shared_ptr<Communicator>&,
               const StringSeq&) override;

    void stop() override;
};

extern "C"
{

//
// Factory function
//
ICE_DECLSPEC_EXPORT ::IceBox::Service*
create(shared_ptr<Communicator>)
{
    return new ServiceI;
}

}

void
ServiceI::start(const string& name,
                const shared_ptr<Communicator>& communicator,
                const StringSeq&)
{
    auto properties = communicator->getProperties();
    auto adapter = communicator->createObjectAdapter(name);
    adapter->add(make_shared<TestI>(adapter, properties), stringToIdentity(properties->getProperty(name + ".Identity")));
    adapter->activate();
}

void
ServiceI::stop()
{
}
