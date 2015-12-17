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

DEFINE_TEST("client")

#ifdef _MSC_VER
// For 'Ice::Communicator::addObjectFactory()' deprecation
#pragma warning( disable : 4996 )
#endif

#if defined(__GNUC__)
// For 'Ice::Communicator::addObjectFactory()' deprecation
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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

};
#endif
class MyObjectFactory : public Ice::ObjectFactory
{
public:
    MyObjectFactory() : _destroyed(false)
    {
    }

    ~MyObjectFactory()
    {
        assert(_destroyed);
    }

    virtual Ice::ValuePtr create(const string& type)
    {
        return ICE_NULLPTR;
    }

    virtual void destroy()
    {
        _destroyed = true;
    }

private:
    bool _destroyed;
};

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
#ifdef ICE_CPP11_MAPPING
    communicator->addValueFactory(makeFactory<BI>(), "::Test::B");
    communicator->addValueFactory(makeFactory<CI>(), "::Test::C");
    communicator->addValueFactory(makeFactory<DI>(), "::Test::D");
    communicator->addValueFactory(makeFactory<EI>(), "::Test::E");
    communicator->addValueFactory(makeFactory<FI>(), "::Test::F");
    communicator->addValueFactory(makeFactory<II>(), "::Test::I");
    communicator->addValueFactory(makeFactory<JI>(), "::Test::J");
    communicator->addValueFactory(makeFactory<HI>(), "::Test::H");
    communicator->addObjectFactory(make_shared<MyObjectFactory>(), "TestOF");
#else
    Ice::ValueFactoryPtr factory = new MyValueFactory;
    communicator->addValueFactory(factory, "::Test::B");
    communicator->addValueFactory(factory, "::Test::C");
    communicator->addValueFactory(factory, "::Test::D");
    communicator->addValueFactory(factory, "::Test::E");
    communicator->addValueFactory(factory, "::Test::F");
    communicator->addValueFactory(factory, "::Test::I");
    communicator->addValueFactory(factory, "::Test::J");
    communicator->addValueFactory(factory, "::Test::H");
    communicator->addObjectFactory(new MyObjectFactory(), "TestOF");
#endif

    InitialPrxPtr allTests(const Ice::CommunicatorPtr&);
    InitialPrxPtr initial = allTests(communicator);
    initial->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#   if defined(__linux)
    Ice::registerIceBT();
#   endif
#endif

    try
    {
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv);
        RemoteConfig rc("Ice/objects", argc, argv, ich.communicator());
        int status = run(argc, argv, ich.communicator());
        rc.finished(status);
        return status;
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
}
