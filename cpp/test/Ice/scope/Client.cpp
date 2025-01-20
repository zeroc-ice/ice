// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
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
