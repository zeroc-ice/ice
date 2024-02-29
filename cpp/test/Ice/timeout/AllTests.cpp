//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <TestI.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace Test;

namespace
{

Ice::ConnectionPtr
connect(const Ice::ObjectPrx& prx)
{
    //
    // Establish connection with the given proxy (which might have a timeout
    // set and might sporadically fail on connection establishment if it's
    // too slow). The loop ensures that the connection is established by retrying
    // in case we can a ConnectTimeoutException
    //
    int nRetry = 10;
    while(--nRetry > 0)
    {
        try
        {
            prx->ice_getConnection(); // Establish connection
            break;
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Can sporadically occur with slow machines
        }
    }
    return prx->ice_getConnection();
}

}

void
allTestsWithController(Test::TestHelper* helper, const ControllerPrx& controller)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string sref = "timeout:" + helper->getTestEndpoint();

    TimeoutPrx timeout(communicator, sref);

    cout << "testing connect timeout... " << flush;
    {
        //
        // Expect ConnectTimeoutException.
        //
        TimeoutPrx to = timeout->ice_timeout(100);
        controller->holdAdapter(-1);
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    {
        //
        // Expect success.
        //
        TimeoutPrx to = timeout->ice_timeout(-1);
        controller->holdAdapter(100);
        try
        {
            to->op();
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    // The sequence needs to be large enough to fill the write/recv buffers
    ByteSeq seq(2000000);

    cout << "testing connection timeout... " << flush;
    {
        //
        // Expect TimeoutException.
        //
        TimeoutPrx to = timeout->ice_timeout(250);
        connect(to);
        controller->holdAdapter(-1);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    {
        //
        // Expect success.
        //
        TimeoutPrx to = timeout->ice_timeout(2000);
        controller->holdAdapter(100);
        try
        {
            ByteSeq seq2(1000000);
            to->sendData(seq2);
        }
        catch(const Ice::TimeoutException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "testing invocation timeout... " << flush;
    {
        Ice::ConnectionPtr connection = timeout->ice_getConnection();
        TimeoutPrx to = timeout->ice_invocationTimeout(100);
        test(connection == to->ice_getConnection());
        try
        {
            to->sleep(1000);
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }
        timeout->ice_ping();
        to = timeout->ice_invocationTimeout(1000);
        test(connection == to->ice_getConnection());
        try
        {
            to->sleep(100);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
            test(false);
        }
        test(connection == to->ice_getConnection());
    }
    {
        //
        // Expect InvocationTimeoutException.
        //
        TimeoutPrx to = timeout->ice_invocationTimeout(100);

        auto f = to->sleepAsync(1000);
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }
        catch(...)
        {
            test(false);
        }
        timeout->ice_ping();
    }
    {
        //
        // Expect success.
        //
        TimeoutPrx to = timeout->ice_invocationTimeout(1000);
        auto f = to->sleepAsync(100);
        try
        {
            f.get();
        }
        catch(...)
        {
            test(false);
        }
    }
    {
        //
        // Backward compatible connection timeouts
        //
        TimeoutPrx to = timeout->ice_invocationTimeout(-2)->ice_timeout(250);
        Ice::ConnectionPtr con = connect(to);
        try
        {
            to->sleep(750);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            try
            {
                con->getInfo();
                test(false);
            }
            catch(const Ice::TimeoutException&)
            {
                // Connection got closed as well.
            }
        }
        timeout->ice_ping();
        try
        {
            con = connect(to);
            to->sleepAsync(750).get();
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            try
            {
                con->getInfo();
                test(false);
            }
            catch(const Ice::TimeoutException&)
            {
                // Connection got closed as well.
            }
        }
        timeout->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing close timeout... " << flush;
    {
        TimeoutPrx to = timeout->ice_timeout(250);
        Ice::ConnectionPtr connection = connect(to);
        controller->holdAdapter(-1);
        connection->close(Ice::ConnectionClose::GracefullyWithWait);
        try
        {
            connection->getInfo(); // getInfo() doesn't throw in the closing state.
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
        while(true)
        {
            try
            {
                connection->getInfo();
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            catch(const Ice::ConnectionManuallyClosedException& ex)
            {
                // Expected.
                test(ex.graceful);
                break;
            }
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    cout << "ok" << endl;

    cout << "testing timeout overrides... " << flush;
    {
        //
        // Test Ice.Override.Timeout. This property overrides all
        // endpoint timeouts.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.ConnectTimeout", "250");
        initData.properties->setProperty("Ice.Override.Timeout", "100");
        Ice::CommunicatorHolder ich(initData);
        TimeoutPrx to(ich.communicator(), sref);
        connect(to);
        controller->holdAdapter(-1);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.

        //
        // Calling ice_timeout() should have no effect.
        //
        to = to->ice_timeout(1000);
        connect(to);
        controller->holdAdapter(-1);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    {
        //
        // Test Ice.Override.ConnectTimeout.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.ConnectTimeout", "250");
        Ice::CommunicatorHolder ich(initData);
        controller->holdAdapter(-1);
        TimeoutPrx to(ich.communicator(), sref);
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.

        //
        // Calling ice_timeout() should have no effect on the connect timeout.
        //
        controller->holdAdapter(-1);
        to = to->ice_timeout(1000);
        try
        {
            to->op();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.

        //
        // Verify that timeout set via ice_timeout() is still used for requests.
        //
        to = to->ice_timeout(250);
        connect(to);
        controller->holdAdapter(-1);
        try
        {
            to->sendData(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
            // Expected.
        }
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    {
        //
        // Test Ice.Override.CloseTimeout.
        //
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Override.CloseTimeout", "100");
        Ice::CommunicatorHolder ich(initData);
        Ice::ConnectionPtr connection = ich->stringToProxy(sref)->ice_getConnection();
        controller->holdAdapter(-1);
        auto now = chrono::steady_clock::now();
        ich.release()->destroy();
        test(chrono::steady_clock::now() - now < chrono::milliseconds(1000));
        controller->resumeAdapter();
        timeout->op(); // Ensure adapter is active.
    }
    cout << "ok" << endl;

    cout << "testing invocation timeouts with collocated calls... " << flush;
    {
        communicator->getProperties()->setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TimeoutCollocated");
        adapter->activate();

        timeout = TimeoutPrx(adapter->addWithUUID(std::make_shared<TimeoutI>()));
        timeout = timeout->ice_invocationTimeout(100);
        try
        {
            timeout->sleep(500);
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        try
        {
            timeout->sleepAsync(500).get();
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        try
        {
            timeout->ice_invocationTimeout(-2)->ice_ping();
            timeout->ice_invocationTimeout(-2)->ice_pingAsync().get();
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }

        TimeoutPrx batchTimeout = timeout->ice_batchOneway();
        batchTimeout->ice_ping();
        batchTimeout->ice_ping();
        batchTimeout->ice_ping();

        // Keep the server thread pool busy.
        timeout->ice_invocationTimeout(-1)->sleepAsync(500);
        try
        {
            batchTimeout->ice_flushBatchRequestsAsync().get();
            test(false);
        }
        catch(const Ice::InvocationTimeoutException&)
        {
        }

        adapter->destroy();
    }
    cout << "ok" << endl;

    controller->shutdown();
}

void
allTests(Test::TestHelper* helper)
{
    ControllerPrx controller(helper->communicator(), "controller:" + helper->getTestEndpoint(1));

    try
    {
        allTestsWithController(helper, controller);
    }
    catch(const Ice::Exception&)
    {
        // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent the test
        // from hanging on exit in case a connection which disables timeouts is still opened.
        controller->resumeAdapter();
        throw;
    }
}
