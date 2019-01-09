// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
    cout << "test same Slice type name in different scopes... " << flush;
    void allTests(Test::TestHelper*);
    allTests(this);
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
