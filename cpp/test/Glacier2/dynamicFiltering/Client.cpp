// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

class SessionControlClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    SessionControlClient app;

    Ice::InitializationData initData = getTestInitData(argc, argv);

    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    initData.properties->setProperty("Ice.RetryIntervals", "-1");
    initData.properties->setProperty("Ice.Warn.Connections", "0");

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
    Ice::InitializationData initData;
    initData.properties = communicator()->getProperties();
    Ice::CommunicatorPtr controlComm = Ice::initialize(argc, argv, initData);
    TestControllerPrx controller = TestControllerPrx::checkedCast(
        controlComm->stringToProxy("testController:" + getTestEndpoint(communicator(), 2, "tcp")));
    test(controller);
    TestToken currentState;
    TestToken newState;
    currentState.expectedResult = false;
    currentState.config = 0;
    currentState.caseIndex = 0;
    currentState.code = Initial;
    controller->step(0, currentState, newState);
    currentState = newState;
    cout << "ok" << endl;

    cout << "getting router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator(), 10));
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
    router->destroySession();

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
    ObjectPrx processBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                          getTestEndpoint(communicator(), 11));
    Ice::ProcessPrx process = Ice::ProcessPrx::checkedCast(processBase);
    test(process);
    process->shutdown();
    try
    {
        process->ice_ping();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }

    try
    {
        controlComm->destroy();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    return EXIT_SUCCESS;
}
