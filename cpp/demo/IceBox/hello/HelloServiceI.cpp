#include <Ice/Ice.h>
#include <HelloServiceI.h>
#include <HelloI.h>

using namespace std;

extern "C"
{

//
// Factory function
//
::IceBox::ServicePtr
create(::Ice::CommunicatorPtr communicator)
{
    return new HelloServiceI;
}

}

HelloServiceI::HelloServiceI()
{
    cout << "HelloServiceI()" << endl;
}

HelloServiceI::~HelloServiceI()
{
    cout << "~HelloServiceI()" << endl;
}

void
HelloServiceI::init(const string& name,
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
HelloServiceI::start()
{
    cout << "HelloServiceI::start" << endl;
}

void
HelloServiceI::stop()
{
    cout << "HelloServiceI::stop" << endl;
    _adapter->deactivate();
}
