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

DEFINE_TEST("collocated")

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
class MyObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const string& type)
    {
        if(type == "::Test::B")
        {
            return new BI;
        }
        else if(type == "::Test::C")
        {
            return new CI;
        }
        else if(type == "::Test::D")
        {
            return new DI;
        }
        else if(type == "::Test::E")
        {
            return new EI;
        }
        else if(type == "::Test::F")
        {
            return new FI;
        }
        else if(type == "::Test::I")
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

    virtual void destroy()
    {
        // Nothing to do
    }
};
#endif

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
#ifdef ICE_CPP11_MAPPING
    communicator->addObjectFactory(makeFactory<BI>(), "::Test::B");
    communicator->addObjectFactory(makeFactory<CI>(), "::Test::C");
    communicator->addObjectFactory(makeFactory<DI>(), "::Test::D");
    communicator->addObjectFactory(makeFactory<EI>(), "::Test::E");
    communicator->addObjectFactory(makeFactory<FI>(), "::Test::F");
    communicator->addObjectFactory(makeFactory<II>(), "::Test::I");
    communicator->addObjectFactory(makeFactory<JI>(), "::Test::J");
    communicator->addObjectFactory(makeFactory<HI>(), "::Test::H");
#else
    Ice::ObjectFactoryPtr factory = new MyObjectFactory;
    communicator->addObjectFactory(factory, "::Test::B");
    communicator->addObjectFactory(factory, "::Test::C");
    communicator->addObjectFactory(factory, "::Test::D");
    communicator->addObjectFactory(factory, "::Test::E");
    communicator->addObjectFactory(factory, "::Test::F");
    communicator->addObjectFactory(factory, "::Test::I");
    communicator->addObjectFactory(factory, "::Test::J");
    communicator->addObjectFactory(factory, "::Test::H");
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(InitialI, adapter), communicator->stringToIdentity("initial"));
#ifndef ICE_CPP11_MAPPING
    UnexpectedObjectExceptionTestIPtr uoet = new UnexpectedObjectExceptionTestI;
    adapter->add(uoet, communicator->stringToIdentity("uoet"));
#endif
    InitialPrxPtr allTests(const Ice::CommunicatorPtr&);
    InitialPrxPtr initial = allTests(communicator);
    // We must call shutdown even in the collocated case for cyclic dependency cleanup
    initial->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
