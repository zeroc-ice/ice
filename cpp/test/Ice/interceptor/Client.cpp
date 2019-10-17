//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    void testInterceptorExceptions(const Test::MyObjectPrxPtr&);
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

    cout << "testing exceptions raised by the interceptor... " << flush;
    testInterceptorExceptions(prx);
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
    {
        Ice::Context ctx;
        ctx["retry"] = "yes";
        for(int i = 0; i < 10; ++i)
        {
            test(prx->amdAdd(33, 12, ctx) == 45);
            test(interceptor->getLastOperation() == "amdAdd");
            test(!interceptor->getLastStatus());
        }
    }
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

    cout << "testing exceptions raised by the interceptor... " << flush;
    testInterceptorExceptions(prx);
    cout << "ok" << endl;
}

void
Client::testInterceptorExceptions(const Test::MyObjectPrxPtr& prx)
{
    vector<pair<string, string> > exceptions;
    exceptions.push_back(make_pair("raiseBeforeDispatch", "user"));
    exceptions.push_back(make_pair("raiseBeforeDispatch", "notExist"));
    exceptions.push_back(make_pair("raiseBeforeDispatch", "system"));
    exceptions.push_back(make_pair("raiseAfterDispatch", "user"));
    exceptions.push_back(make_pair("raiseAfterDispatch", "notExist"));
    exceptions.push_back(make_pair("raiseAfterDispatch", "system"));
    for(vector<pair<string, string> >::const_iterator p = exceptions.begin(); p != exceptions.end(); ++p)
    {
        Ice::Context ctx;
        ctx[p->first] = p->second;
        try
        {
            prx->ice_ping(ctx);
            test(false);
        }
        catch(const Ice::UnknownUserException&)
        {
            test(p->second == "user");
        }
        catch(const Ice::ObjectNotExistException&)
        {
            test(p->second == "notExist");
        }
        catch(const Ice::UnknownException&)
        {
            test(p->second == "system"); // non-collocated
        }
        catch(const MySystemException&)
        {
            test(p->second == "system"); // collocated
        }
        {
            Ice::ObjectPrxPtr batch = prx->ice_batchOneway();
            batch->ice_ping(ctx);
            batch->ice_ping();
            batch->ice_flushBatchRequests();

            // Force the last batch request to be dispatched by the server thread using invocation timeouts
            // This is required to preven threading issue with the test interceptor implementation which
            // isn't thread safe
            prx->ice_invocationTimeout(10000)->ice_ping();
        }
    }
}

DEFINE_TEST(Client)
