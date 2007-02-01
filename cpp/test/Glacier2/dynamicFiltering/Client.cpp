// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <Test.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Test;

static Ice::InitializationData initData;

class SessionControlClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    initData.properties = Ice::createProperties(argc, argv);
    
    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    initData.properties->setProperty("Ice.RetryIntervals", "-1");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
        
    SessionControlClient app;
    return app.main(argc, argv, initData);
}

int
SessionControlClient::run(int argc, char* argv[])
{
    //
    // We initialize the controller on a separate port because we want
    // to bypass the router for test control operations.
    //
    cout << "accessing test controller... " << flush;
    Ice::CommunicatorPtr controlComm = Ice::initialize(argc, argv, initData);
    TestControllerPrx controller = TestControllerPrx::checkedCast(
        controlComm->stringToProxy("testController:tcp -p 12013"));
    test(controller);
    TestToken currentState;
    TestToken newState;
    currentState.code = Initial;
    controller->step(0, currentState, newState);
    currentState = newState;
    cout << "ok" << endl;
    
    cout << "getting router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:default -p 12347");
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    communicator()->setDefaultRouter(router);
    cout << "ok" << endl;

    Glacier2::SessionPrx sessionBase = router->createSession("userid", "abc123");
    Test::TestSessionPrx currentSession = Test::TestSessionPrx::checkedCast(sessionBase);

    bool printOk = false;
    while(currentState.code == Running)
    {
        controller->step(currentSession, currentState, newState);
        currentState = newState;

        if(currentState.code != Running)
        {
            cout << "ok" << endl;
            break;
        }

        //
        // If we are running the first case for this configuration, print the configuration description.
        //
        if(currentState.caseIndex == 0)
        {
            if(printOk)
            {
                cout << "ok" << endl;
            }
            else
            {
                printOk = true;
            }
            cout << currentState.description << "... " << flush;
        }

        if(currentState.expectedResult)
        {
            BackendPrx prx = BackendPrx::uncheckedCast(communicator()->stringToProxy(currentState.testReference));
            try
            {
                prx->check();
            }
            catch(const Exception& ex)
            {
                cerr << ex << endl;
                test(false);
            }
        }
        else
        {
            BackendPrx prx = BackendPrx::uncheckedCast(communicator()->stringToProxy(currentState.testReference));
            try
            {
                prx->check();
                test(false);
            }
            catch(const ObjectNotExistException&)
            {
            }
            catch(const Exception& ex)
            {
                cerr << ex << endl;
                test(false);
            }
        }
    }

    //
    // Cleanup.
    //
    try
    {
        router->destroySession();
    }
    catch(const ConnectionLostException&)
    {
    }
    catch(const CloseConnectionException&)
    {
    }

    cout << "testing shutdown... " << flush;

    try
    {
        //
        // Shut down the test server.
        //
        currentSession = Test::TestSessionPrx::uncheckedCast(router->createSession("userid", "abc123"));
        currentSession->shutdown();
    }
    catch(const Glacier2::CannotCreateSessionException& ex)
    {
        cerr << ex.reason << endl;
        throw ex;
    }

    //
    // Shut down the router.
    //
    communicator()->setDefaultRouter(0);
    ObjectPrx adminBase = communicator()->stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12348 -t 10000");
    Glacier2::AdminPrx admin = Glacier2::AdminPrx::checkedCast(adminBase);
    test(admin);
    admin->shutdown();
    try
    {
        admin->ice_ping();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }

    return EXIT_SUCCESS;
}
