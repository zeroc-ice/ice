// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

//
// For 'Ice::Communicator::addObjectFactory()' deprecation
//
#if defined(_MSC_VER)
#   pragma warning( disable : 4996 )
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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

    virtual Ice::ValuePtr create(const string&)
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

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
#ifndef ICE_CPP11_MAPPING
    properties->setProperty("Ice.CollectObjects", "1");
#endif
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

#ifdef ICE_CPP11_MAPPING
    communicator->getValueFactoryManager()->add(makeFactory<BI>(), "::Test::B");
    communicator->getValueFactoryManager()->add(makeFactory<CI>(), "::Test::C");
    communicator->getValueFactoryManager()->add(makeFactory<DI>(), "::Test::D");
    communicator->getValueFactoryManager()->add(makeFactory<EI>(), "::Test::E");
    communicator->getValueFactoryManager()->add(makeFactory<FI>(), "::Test::F");
    communicator->getValueFactoryManager()->add(makeFactory<II>(), "::Test::I");
    communicator->getValueFactoryManager()->add(makeFactory<JI>(), "::Test::J");
    communicator->getValueFactoryManager()->add(makeFactory<HI>(), "::Test::H");
    communicator->addObjectFactory(make_shared<MyObjectFactory>(), "TestOF");
#else
    Ice::ValueFactoryPtr factory = new MyValueFactory;
    communicator->getValueFactoryManager()->add(factory, "::Test::B");
    communicator->getValueFactoryManager()->add(factory, "::Test::C");
    communicator->getValueFactoryManager()->add(factory, "::Test::D");
    communicator->getValueFactoryManager()->add(factory, "::Test::E");
    communicator->getValueFactoryManager()->add(factory, "::Test::F");
    communicator->getValueFactoryManager()->add(factory, "::Test::I");
    communicator->getValueFactoryManager()->add(factory, "::Test::J");
    communicator->getValueFactoryManager()->add(factory, "::Test::H");
    communicator->addObjectFactory(new MyObjectFactory(), "TestOF");
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(InitialI, adapter), Ice::stringToIdentity("initial"));
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    adapter->add(ICE_MAKE_SHARED(UnexpectedObjectExceptionTestI), Ice::stringToIdentity("uoet"));
    InitialPrxPtr allTests(Test::TestHelper*);
    InitialPrxPtr initial = allTests(this);
    // We must call shutdown even in the collocated case for cyclic dependency cleanup
    initial->shutdown();
}

DEFINE_TEST(Collocated)
