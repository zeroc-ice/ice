#include <Ice/Ice.h>
#include <HelloServiceI.h>
#include <HelloI.h>

using namespace std;

extern "C"
{

//
// Factory function
//
HELLO_API ::IceBox::Service*
create(::Ice::CommunicatorPtr communicator)
{
    return new HelloServiceI;
}

}

HelloServiceI::HelloServiceI()
{
}

HelloServiceI::~HelloServiceI()
{
}

void
HelloServiceI::start(const string& name,
		     const ::Ice::CommunicatorPtr& communicator,
		     const ::Ice::PropertiesPtr& properties,
		     const ::Ice::StringSeq& args)
{
    _adapter = communicator->createObjectAdapter(name + "Adapter");
    ::Ice::ObjectPtr object = new HelloI(communicator);
    _adapter->add(object, ::Ice::stringToIdentity("hello"));
    _adapter->activate();
}

void
HelloServiceI::stop()
{
    _adapter->deactivate();
}
