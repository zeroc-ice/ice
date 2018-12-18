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
#include <Test.h>
#include <MyObjectI.h>
#include <InterceptorI.h>
#include <AMDInterceptorI.h>
#include <iostream>

#ifndef _WIN32
//
// SIGPIPE test
//
#   include <signal.h>

extern "C" void testAction(int)
{
    test(false);
}

#endif

using namespace std;

class Client : public Test::TestHelper
{
public:

    virtual void run(int, char**);

private:

    void runTest(const Test::MyObjectPrxPtr&, const InterceptorIPtr&);
    void runAmdTest(const Test::MyObjectPrxPtr&, const AMDInterceptorIPtr&);
};

void
Client::run(int argc, char* argv[])
{
#ifndef _WIN32
    //
    // Set SIGPIPE action
    //
    struct sigaction action;
    action.sa_handler = &testAction;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, 0);
#endif
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Dispatch", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    //
    // Create OA and servants
    //
    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");

    Ice::ObjectPtr servant = ICE_MAKE_SHARED(MyObjectI);
    InterceptorIPtr interceptor = ICE_MAKE_SHARED(InterceptorI, servant);
    AMDInterceptorIPtr amdInterceptor = ICE_MAKE_SHARED(AMDInterceptorI, servant);

    Test::MyObjectPrxPtr prx = ICE_UNCHECKED_CAST(Test::MyObjectPrx, oa->addWithUUID(interceptor));
    Test::MyObjectPrxPtr prxForAMD = ICE_UNCHECKED_CAST(Test::MyObjectPrx, oa->addWithUUID(amdInterceptor));

    cout << "Collocation optimization on" << endl;
    runTest(prx, interceptor);

    cout << "Now with AMD" << endl;
    runAmdTest(prxForAMD, amdInterceptor);

    oa->activate(); // Only necessary for non-collocation optimized tests

    cout << "Collocation optimization off" << endl;
    interceptor->clear();
    prx = ICE_UNCHECKED_CAST(Test::MyObjectPrx, prx->ice_collocationOptimized(false));
    runTest(prx, interceptor);

    cout << "Now with AMD" << endl;
    amdInterceptor->clear();
    prxForAMD = ICE_UNCHECKED_CAST(Test::MyObjectPrx, prxForAMD->ice_collocationOptimized(false));
    runAmdTest(prxForAMD, amdInterceptor);
#ifndef _WIN32
    //
    // Check SIGPIPE was properly reset to old action
    //
    struct sigaction newAction;
    sigaction(SIGPIPE, 0, &newAction);
    test(action.sa_handler == &testAction);
#endif
}

void
Client::runTest(const Test::MyObjectPrxPtr& prx, const InterceptorIPtr& interceptor)
{
    cout << "testing simple interceptor... " << flush;
    test(interceptor->getLastOperation().empty());
    prx->ice_ping();
    test(interceptor->getLastOperation() == "ice_ping");
    test(interceptor->getLastStatus());
    string typeId = prx->ice_id();
    test(interceptor->getLastOperation() == "ice_id");
    test(interceptor->getLastStatus());
    test(prx->ice_isA(typeId));
    test(interceptor->getLastOperation() == "ice_isA");
    test(interceptor->getLastStatus());
    test(prx->add(33, 12) == 45);
    test(interceptor->getLastOperation() == "add");
    test(interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing retry... " << flush;
    test(prx->addWithRetry(33, 12) == 45);
    test(interceptor->getLastOperation() == "addWithRetry");
    test(interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing user exception... " << flush;
    try
    {
        prx->badAdd(33, 12);
        test(false);
    }
    catch(const Test::InvalidInputException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "badAdd");
    test(interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing ONE... " << flush;

    interceptor->clear();
    try
    {
        prx->notExistAdd(33, 12);
        test(false);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "notExistAdd");
    cout << "ok" << endl;
    cout << "testing system exception... " << flush;
    interceptor->clear();
    try
    {
        prx->badSystemAdd(33, 12);
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
        test(!prx->ice_isCollocationOptimized());
    }
    catch(const MySystemException&)
    {
        test(prx->ice_isCollocationOptimized());
    }
    catch(...)
    {
        test(false);
    }
    test(interceptor->getLastOperation() == "badSystemAdd");
    cout << "ok" << endl;

    cout << "testing simple AMD... " << flush;
    test(prx->amdAdd(33, 12) == 45);
    test(interceptor->getLastOperation() == "amdAdd");
    test(!interceptor->getLastStatus());
    cout << "ok" << endl;
}

void
Client::runAmdTest(const Test::MyObjectPrxPtr& prx, const AMDInterceptorIPtr& interceptor)
{
    cout << "testing simple interceptor... " << flush;
    test(interceptor->getLastOperation().empty());
    test(prx->amdAdd(33, 12) == 45);
    test(interceptor->getLastOperation() == "amdAdd");
    test(!interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing retry... " << flush;
    test(prx->amdAddWithRetry(33, 12) == 45);
    test(interceptor->getLastOperation() == "amdAddWithRetry");
    test(!interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing user exception... " << flush;
    try
    {
        prx->amdBadAdd(33, 12);
        test(false);
    }
    catch(const Test::InvalidInputException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "amdBadAdd");
    test(!interceptor->getLastStatus());
    cout << "ok" << endl;
    cout << "testing ONE... " << flush;
    interceptor->clear();
    try
    {
        prx->amdNotExistAdd(33, 12);
        test(false);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "amdNotExistAdd");
    test(!interceptor->getLastStatus());

    test(dynamic_cast<Ice::ObjectNotExistException*>(interceptor->getException()) != 0);

    cout << "ok" << endl;
    cout << "testing system exception... " << flush;
    interceptor->clear();
    try
    {
        prx->amdBadSystemAdd(33, 12);
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
        test(!prx->ice_isCollocationOptimized());
    }
    catch(const MySystemException&)
    {
        test(prx->ice_isCollocationOptimized());
    }
    test(interceptor->getLastOperation() == "amdBadSystemAdd");
    test(!interceptor->getLastStatus());
    test(dynamic_cast<MySystemException*>(interceptor->getException()) != 0);
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
