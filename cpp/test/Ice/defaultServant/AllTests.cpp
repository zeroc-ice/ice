// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
    oa->activate();

    Ice::ObjectPtr servant = std::make_shared<MyObjectI>();

    //
    // Register default servant with category "foo"
    //
    oa->addDefaultServant(servant, "foo");

    //
    // Start test
    //
    cout << "testing single category... " << flush;

    Ice::ObjectPtr r = oa->findDefaultServant("foo");
    test(r == servant);

    r = oa->findDefaultServant("bar");
    test(r == nullptr);

    Ice::Identity identity;
    identity.category = "foo";

    string names[] = {"foo", "bar", "x", "y", "abcdefg"};

    int idx;

    for (idx = 0; idx < 5; ++idx)
    {
        identity.name = names[idx];
        auto prx = oa->createProxy<MyObjectPrx>(identity);
        prx->ice_ping();
        test(prx->getName() == names[idx]);
    }

    identity.name = "ObjectNotExist";
    auto prx = oa->createProxy<MyObjectPrx>(identity);
    try
    {
        prx->ice_ping();
        test(false);
    }
    catch (const Ice::ObjectNotExistException&)
    {
        // Expected
    }

    try
    {
        prx->getName();
        test(false);
    }
    catch (const Ice::ObjectNotExistException&)
    {
        // Expected
    }

    identity.name = "FacetNotExist";
    prx = oa->createProxy<MyObjectPrx>(identity);
    try
    {
        prx->ice_ping();
        test(false);
    }
    catch (const Ice::FacetNotExistException&)
    {
        // Expected
    }

    try
    {
        prx->getName();
        test(false);
    }
    catch (const Ice::FacetNotExistException&)
    {
        // Expected
    }

    identity.category = "bar";
    for (idx = 0; idx < 5; idx++)
    {
        identity.name = names[idx];
        prx = oa->createProxy<MyObjectPrx>(identity);

        try
        {
            prx->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
            // Expected
        }

        try
        {
            prx->getName();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
            // Expected
        }
    }

    oa->removeDefaultServant("foo");
    identity.category = "foo";
    prx = oa->createProxy<MyObjectPrx>(identity);
    try
    {
        prx->ice_ping();
    }
    catch (const Ice::ObjectNotExistException&)
    {
        // Expected
    }

    cout << "ok" << endl;

    cout << "testing default category... " << flush;

    oa->addDefaultServant(servant, "");

    r = oa->findDefaultServant("bar");
    test(r == nullptr);

    r = oa->findDefaultServant("");
    test(r == servant);

    for (idx = 0; idx < 5; ++idx)
    {
        identity.name = names[idx];
        prx = oa->createProxy<MyObjectPrx>(identity);
        prx->ice_ping();
        test(prx->getName() == names[idx]);
    }

    cout << "ok" << endl;
}
