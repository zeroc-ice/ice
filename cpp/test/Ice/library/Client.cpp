// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

ICE_DECLSPEC_IMPORT void allTests(const Ice::ObjectAdapterPtr&);

#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("alltests"))
#endif

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
    // Collocated-only OA
    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapter("");

    oa->activate();
    allTests(oa);
}

DEFINE_TEST(Client)
