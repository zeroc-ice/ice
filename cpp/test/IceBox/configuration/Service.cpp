// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
#ifdef ICE_CPP11_MAPPING
create(const shared_ptr<Communicator>&)
#else
create(CommunicatorPtr)
#endif
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
ServiceI::start(const string& name, const CommunicatorPtr& communicator, const StringSeq& args)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter(name + "OA");
    adapter->add(ICE_MAKE_SHARED(TestI, args), stringToIdentity("test"));
    adapter->activate();
}

void
ServiceI::stop()
{
}
