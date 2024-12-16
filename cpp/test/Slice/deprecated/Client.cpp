// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace DeprecatedTest;

// Make sure we can implement a deprecated interface without warning.
class MyInterfaceI final : public MyInterface
{
public:
    void opInt(std::int32_t, const Ice::Current&) final {}
};

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing deprecated definitions... " << flush;

    MyOtherInterfacePrx prx{communicator, "foo"}; // MyOtherInterfacePrx is not deprecated

    try
    {
        prx->opInt(5); // non-deprecated op
        test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
        // expected
    }

    cout << "ok" << endl;
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv);
    allTests(ich.communicator());
}

DEFINE_TEST(Client)
