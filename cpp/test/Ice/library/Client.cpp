// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>

ICE_DECLSPEC_IMPORT void
allTests(const Ice::ObjectAdapterPtr&);

#if defined(_MSC_VER)
#   pragma comment(lib, ICE_LIBNAME("alltests"))
#endif

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
    // Collocated-only OA
    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapter("");

    oa->activate();
    allTests(oa);
}

DEFINE_TEST(Client)
