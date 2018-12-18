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

using namespace std;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr d = ICE_MAKE_SHARED(DI);
    adapter->add(d, Ice::stringToIdentity("d"));
    adapter->addFacet(d, Ice::stringToIdentity("d"), "facetABCD");
    Ice::ObjectPtr f = ICE_MAKE_SHARED(FI);
    adapter->addFacet(f, Ice::stringToIdentity("d"), "facetEF");
    Ice::ObjectPtr h = ICE_MAKE_SHARED(HI);
    adapter->addFacet(h, Ice::stringToIdentity("d"), "facetGH");

    GPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
