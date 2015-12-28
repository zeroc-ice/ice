// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;
using namespace Test;

#ifdef ICE_CPP11_MAPPING
template<typename T>
function<shared_ptr<T>(const string&)> makeFactory()
{
    return [](const string&)
        {
            return make_shared<T>();
        };
}
#else
class MyValueFactory : public Ice::ValueFactory
{
public:

    virtual Ice::ObjectPtr create(const string& type)
    {
        if(type == "::Test::I")
        {
            return new II;
        }
        else if(type == "::Test::J")
        {
            return new JI;
        }
        else if(type == "::Test::H")
        {
            return new HI;
        }

        assert(false); // Should never be reached
        return 0;
    }

};
#endif

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
#ifdef ICE_CPP11_MAPPING
    communicator->addValueFactory(makeFactory<II>(), "::Test::I");
    communicator->addValueFactory(makeFactory<JI>(), "::Test::J");
    communicator->addValueFactory(makeFactory<HI>(), "::Test::H");
#else
    Ice::ValueFactoryPtr factory = new MyValueFactory;
    communicator->addValueFactory(factory, "::Test::I");
    communicator->addValueFactory(factory, "::Test::J");
    communicator->addValueFactory(factory, "::Test::H");
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(InitialI, adapter), communicator->stringToIdentity("initial"));
    adapter->add(ICE_MAKE_SHARED(TestIntfI), communicator->stringToIdentity("test"));
    
#ifdef ICE_CPP11_MAPPING
    // TODO
#else
    UnexpectedObjectExceptionTestIPtr uoet = new UnexpectedObjectExceptionTestI;
    adapter->add(uoet, communicator->stringToIdentity("uoet"));
#endif
    adapter->activate();
    TEST_READY
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    try
    {
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
