//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    //
    // In this test, we need at least two threads in the
    // client side thread pool for nested AMI.
    //
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.ThreadPool.Client.Size", "2");
    properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
    properties->setProperty("Ice.BatchAutoFlushSize", "100");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    Test::MyClassPrxPtr allTests(Test::TestHelper*);
    Test::MyClassPrxPtr myClass = allTests(this);

    myClass->shutdown();
    cout << "testing server shutdown... " << flush;
    try
    {
#ifdef _WIN32
        myClass = myClass->ice_timeout(100); // Workaround to speed up testing
#endif
        myClass->opVoid();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
}

DEFINE_TEST(Client)
