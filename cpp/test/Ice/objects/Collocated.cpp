//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

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

    virtual Ice::ValuePtr create(const string& type)
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

        assert(false); // Should never be reached
        return 0;
    }

};
#endif

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.AcceptClassCycles", "1");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

#ifdef ICE_CPP11_MAPPING
    communicator->getValueFactoryManager()->add(makeFactory<BI>(), "::Test::B");
    communicator->getValueFactoryManager()->add(makeFactory<CI>(), "::Test::C");
    communicator->getValueFactoryManager()->add(makeFactory<DI>(), "::Test::D");
    communicator->getValueFactoryManager()->add(makeFactory<EI>(), "::Test::E");
    communicator->getValueFactoryManager()->add(makeFactory<FI>(), "::Test::F");
#else
    Ice::ValueFactoryPtr factory = new MyValueFactory;
    communicator->getValueFactoryManager()->add(factory, "::Test::B");
    communicator->getValueFactoryManager()->add(factory, "::Test::C");
    communicator->getValueFactoryManager()->add(factory, "::Test::D");
    communicator->getValueFactoryManager()->add(factory, "::Test::E");
    communicator->getValueFactoryManager()->add(factory, "::Test::F");
#endif

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(InitialI, adapter), Ice::stringToIdentity("initial"));
    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    adapter->add(ICE_MAKE_SHARED(F2I), Ice::stringToIdentity("F21"));
    adapter->add(ICE_MAKE_SHARED(UnexpectedObjectExceptionTestI), Ice::stringToIdentity("uoet"));
    InitialPrxPtr allTests(Test::TestHelper*);
    InitialPrxPtr initial = allTests(this);
    // We must call shutdown even in the collocated case for cyclic dependency cleanup
    initial->shutdown();
}

DEFINE_TEST(Collocated)
