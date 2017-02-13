// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr d = ICE_MAKE_SHARED(DI);
    adapter->add(d, Ice::stringToIdentity("d"));
    adapter->addFacet(d, Ice::stringToIdentity("d"), "facetABCD");
    Ice::ObjectPtr f = ICE_MAKE_SHARED(FI);
    adapter->addFacet(f, Ice::stringToIdentity("d"), "facetEF");
    Ice::ObjectPtr h = ICE_MAKE_SHARED(HI, communicator);
    adapter->addFacet(h, Ice::stringToIdentity("d"), "facetGH");

    GPrxPtr allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
