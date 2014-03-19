// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <MyObjectI.h>
#include <InterceptorI.h>
#include <AMDInterceptorI.h>
#include <iostream>

DEFINE_TEST("client")

#ifndef _WIN32
//
// SIGPIPE test
//
#   include <signal.h>
#endif


using namespace std;

class Client : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    
private:

    int run(const Test::MyObjectPrx&, const InterceptorIPtr&);
    int runAmd(const Test::MyObjectPrx&, const AMDInterceptorIPtr&); 
};

#ifndef _WIN32
extern "C" void testAction(int)
{
    test(false);
}
#endif

int
main(int argc, char* argv[])
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

    Client app;
    int result = app.main(argc, argv);

#ifndef _WIN32
//
// Check SIGPIPE was properly reset to old action
//
    struct sigaction newAction;
    sigaction(SIGPIPE, 0, &newAction);
    test(action.sa_handler == &testAction);
#endif

    return result;
}

int
Client::run(int, char*[])
{

#ifndef _WIN32
//
// Check SIGPIPE is now SIG_IGN
//
    struct sigaction action;
    sigaction(SIGPIPE, 0, &action);
    test(action.sa_handler == SIG_IGN);
#endif

    //
    // Create OA and servants  
    //  
    Ice::ObjectAdapterPtr oa = communicator()->createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
    
    Ice::ObjectPtr servant = new MyObjectI;
    InterceptorIPtr interceptor = new InterceptorI(servant);
    
    Test::MyObjectPrx prx = Test::MyObjectPrx::uncheckedCast(oa->addWithUUID(interceptor));
    
    oa->activate();
       
    cout << "Collocation optimization on" << endl;
    int rs = run(prx, interceptor);
    if(rs == 0)
    {
        cout << "Collocation optimization off" << endl;
        interceptor->clear();
        prx = Test::MyObjectPrx::uncheckedCast(prx->ice_collocationOptimized(false));
        rs = run(prx, interceptor);
        
        if(rs == 0)
        {
            cout << "Now with AMD" << endl;
            AMDInterceptorIPtr amdInterceptor = new AMDInterceptorI(servant);
            prx = Test::MyObjectPrx::uncheckedCast(oa->addWithUUID(amdInterceptor));
            prx = Test::MyObjectPrx::uncheckedCast(prx->ice_collocationOptimized(false));
            
            rs = runAmd(prx, amdInterceptor);
        }
    }
    return rs;
}


int
Client::run(const Test::MyObjectPrx& prx, const InterceptorIPtr& interceptor)
{
    cout << "testing simple interceptor... " << flush;
    test(interceptor->getLastOperation().empty());
    prx->ice_ping();
    test(interceptor->getLastOperation() == "ice_ping");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    string typeId = prx->ice_id();
    test(interceptor->getLastOperation() == "ice_id");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    test(prx->ice_isA(typeId));
    test(interceptor->getLastOperation() == "ice_isA");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    test(prx->add(33, 12) == 45);
    test(interceptor->getLastOperation() == "add");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    cout << "ok" << endl;
    cout << "testing retry... " << flush;
    test(prx->addWithRetry(33, 12) == 45);
    test(interceptor->getLastOperation() == "addWithRetry");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
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
    test(interceptor->getLastStatus() == Ice::DispatchUserException);
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
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch(...)
    {
        test(false);
    }
    test(interceptor->getLastOperation() == "badSystemAdd");
    cout << "ok" << endl;
    if(!prx->ice_isCollocationOptimized())
    {
        cout << "testing simple AMD... " << flush;
        test(prx->amdAdd(33, 12) == 45);
        test(interceptor->getLastOperation() == "amdAdd");
        test(interceptor->getLastStatus() == Ice::DispatchAsync);
        cout << "ok" << endl;
    }
    return 0;
}

int
Client::runAmd(const Test::MyObjectPrx& prx, const AMDInterceptorIPtr& interceptor)
{
    cout << "testing simple interceptor... " << flush;
    test(interceptor->getLastOperation().empty());
    test(prx->amdAdd(33, 12) == 45);
    test(interceptor->getLastOperation() == "amdAdd");
    test(interceptor->getLastStatus() == Ice::DispatchAsync);
    test(interceptor->getActualStatus() == Ice::DispatchOK);
    cout << "ok" << endl;
    cout << "testing retry... " << flush;
    test(prx->amdAddWithRetry(33, 12) == 45);
    test(interceptor->getLastOperation() == "amdAddWithRetry");
    test(interceptor->getLastStatus() == Ice::DispatchAsync);
    test(interceptor->getActualStatus() == Ice::DispatchOK);
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
    test(interceptor->getLastStatus() == Ice::DispatchAsync);
    test(interceptor->getActualStatus() == Ice::DispatchUserException);
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
    test(interceptor->getLastStatus() == Ice::DispatchAsync);
    test(interceptor->getActualStatus() == Ice::DispatchAsync);
    test(dynamic_cast<Ice::ObjectNotExistException*>(interceptor->getException()) != 0);
    cout << "ok" << endl;
    cout << "testing system exception... " << flush;
    interceptor->clear();
    try
    {
        prx->amdBadSystemAdd(33, 12);
        test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
        test(!prx->ice_isCollocationOptimized());
    }
    test(interceptor->getLastOperation() == "amdBadSystemAdd");
    test(interceptor->getLastStatus() == Ice::DispatchAsync);
    test(interceptor->getActualStatus() == Ice::DispatchAsync);
    test(dynamic_cast<Ice::InitializationException*>(interceptor->getException()) != 0);
    cout << "ok" << endl;
    return 0;
}
