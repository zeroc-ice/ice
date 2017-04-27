// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
function<shared_ptr<T>(string)> makeFactory()
{
    return [](string)
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
    communicator->getValueFactoryManager()->add(makeFactory<II>(), "::Test::I");
    communicator->getValueFactoryManager()->add(makeFactory<JI>(), "::Test::J");
    communicator->getValueFactoryManager()->add(makeFactory<HI>(), "::Test::H");
#else
    Ice::ValueFactoryPtr factory = new MyValueFactory;
    communicator->getValueFactoryManager()->add(factory, "::Test::I");
    communicator->getValueFactoryManager()->add(factory, "::Test::J");
    communicator->getValueFactoryManager()->add(factory, "::Test::H");
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(InitialI, adapter), Ice::stringToIdentity("initial"));
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));

    adapter->add(ICE_MAKE_SHARED(UnexpectedObjectExceptionTestI), Ice::stringToIdentity("uoet"));
    adapter->activate();
    TEST_READY
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
