// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <stdlib.h>
#include <TestHelper.h>
#include <TestSuite.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int, char**)
{
    initializeTestSuite();

    for(list<TestBasePtr>::const_iterator p = allTests.begin(); p != allTests.end(); ++p)
    {
        (*p)->start();
    }
}

DEFINE_TEST(Client)
