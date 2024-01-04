//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Test.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlClient final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
SessionControlClient::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");
    properties->setProperty("Ice.Warn.Connections", "0");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    //
    // We initialize the controller on a separate port because we want
    // to bypass the router for test control operations.
    //
    cout << "accessing test controller... " << flush;
    Ice::InitializationData initData;
    initData.properties = communicator->getProperties();
    auto controlComm = Ice::initialize(argc, argv, initData);
    auto controller = checkedCast<TestControllerPrx>(
        controlComm->stringToProxy("testController:" + getTestEndpoint(2, "tcp")));
    test(controller);
    TestToken currentState;
    TestToken newState;
    currentState.expectedResult = false;
    currentState.config = 0;
    currentState.caseIndex = 0;
    currentState.code = StateCode::Initial;
    controller->step(0, currentState, newState);
    currentState = newState;
    cout << "ok" << endl;

    cout << "getting router... " << flush;
    auto routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
    test(router);
    communicator->setDefaultRouter(router);
    cout << "ok" << endl;

    auto sessionBase = router->createSession("userid", "abc123");
    auto currentSession = checkedCast<Test::TestSessionPrx>(sessionBase);

    bool printOk = false;
    while(currentState.code == StateCode::Running)
    {
        controller->step(currentSession, currentState, newState);
        currentState = newState;

        if(currentState.code != StateCode::Running)
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
            auto prx = uncheckedCast<BackendPrx>(communicator->stringToProxy(currentState.testReference));
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
            auto prx = uncheckedCast<BackendPrx>(communicator->stringToProxy(currentState.testReference));
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
        currentSession = uncheckedCast<Test::TestSessionPrx>(router->createSession("userid", "abc123"));
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
    communicator->setDefaultRouter(0);
    auto processBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
    auto process = checkedCast<Ice::ProcessPrx>(processBase);
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
}

DEFINE_TEST(SessionControlClient)
