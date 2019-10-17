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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    cout << "test using same type name in different Slice modules... " << flush;
    void allTests(Test::TestHelper*);
    allTests(this);
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
