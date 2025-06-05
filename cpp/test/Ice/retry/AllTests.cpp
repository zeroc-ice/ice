// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "InstrumentationI.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

class CallbackBase
{
public:
    CallbackBase() = default;

    virtual ~CallbackBase() = default;

    void check()
    {
        unique_lock lock(_mutex);
        _condition.wait(lock, [this] { return _called; });
        _called = false;
    }

protected:
    void called()
    {
        lock_guard lock(_mutex);
        assert(!_called);
        _called = true;
        _condition.notify_one();
    }

private:
    bool _called{false};
    mutex _mutex;
    condition_variable _condition;
};

class CallbackSuccess : public CallbackBase
{
public:
    void response() { called(); }

    void exception(const Ice::Exception&) { test(false); }
};
using CallbackSuccessPtr = shared_ptr<CallbackSuccess>;

class CallbackFail : public CallbackBase
{
public:
    void response() { test(false); }

    void exception(const Ice::Exception& ex)
    {
        test(
            dynamic_cast<const Ice::ConnectionLostException*>(&ex) ||
            dynamic_cast<const Ice::UnknownLocalException*>(&ex));
        called();
    }
};
using CallbackFailPtr = shared_ptr<CallbackFail>;

RetryPrx
allTests(const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2, const string& ref)
{
    RetryPrx retry1(communicator, ref);
    RetryPrx retry2(communicator, ref);

    cout << "calling regular operation with first proxy... " << flush;
    retry1->op(false);
    cout << "ok" << endl;

    testInvocationCount(1);

    cout << "calling operation to kill connection with second proxy... " << flush;
    try
    {
        retry2->op(true);
        test(false);
    }
    catch (const Ice::UnknownLocalException&)
    {
        // Expected with collocation
    }
    catch (const Ice::ConnectionLostException&)
    {
    }
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling regular operation with first proxy again... " << flush;
    retry1->op(false);
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(0);
    cout << "ok" << endl;

    CallbackSuccessPtr cb1 = make_shared<CallbackSuccess>();
    CallbackFailPtr cb2 = make_shared<CallbackFail>();

    cout << "calling regular AMI operation with first proxy... " << flush;
    retry1->opAsync(
        false,
        [cb1]() { cb1->response(); },
        [cb1](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch (const Ice::Exception& ex)
            {
                cb1->exception(ex);
            }
        });
    cb1->check();
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling AMI operation to kill connection with second proxy... " << flush;
    retry2->opAsync(
        true,
        [cb2]() { cb2->response(); },
        [cb2](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch (const Ice::Exception& ex)
            {
                cb2->exception(ex);
            }
        });
    cb2->check();
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling regular AMI operation with first proxy again... " << flush;
    retry1->opAsync(
        false,
        [cb1]() { cb1->response(); },
        [cb1](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch (const Ice::Exception& ex)
            {
                cb1->exception(ex);
            }
        });
    cb1->check();
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "testing idempotent operation... " << flush;
    test(retry1->opIdempotent(4) == 4);
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(4);
    test(retry1->opIdempotentAsync(4).get() == 4);
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(4);
    cout << "ok" << endl;

    if (retry1->ice_getCachedConnection())
    {
        cout << "testing non-idempotent operation with bi-dir proxy... " << flush;
        try
        {
            retry1->ice_fixed(retry1->ice_getCachedConnection())->opIdempotent(4);
        }
        catch (const Ice::Exception&)
        {
        }
        testInvocationCount(1);
        testFailureCount(1);
        testRetryCount(0);
        test(retry1->opIdempotent(4) == 4);
        testInvocationCount(1);
        testFailureCount(0);
        // It succeeded after 3 retry because of the failed opIdempotent on the fixed proxy above
        testRetryCount(3);
        cout << "ok" << endl;
    }

    cout << "testing non-idempotent operation... " << flush;
    try
    {
        retry1->opNotIdempotent();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    try
    {
        retry1->opNotIdempotentAsync().get();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    {
        cout << "testing invocation timeout and retries... " << flush;
        retry2 = RetryPrx(communicator2, retry1->ice_toString());
        retry2->ice_getConnection();
        try
        {
            retry2->ice_invocationTimeout(500)->opIdempotent(4); // No more than 2 retries before timeout kicks-in
            test(false);
        }
        catch (const Ice::InvocationTimeoutException&)
        {
            testRetryCount(2);
            retry2->opIdempotent(-1); // Reset the counter
            testRetryCount(-1);
        }
        try
        {
            // No more than 2 retries before timeout kicks-in
            RetryPrx prx = retry2->ice_invocationTimeout(500);
            prx->opIdempotentAsync(4).get();
            test(false);
        }
        catch (const Ice::InvocationTimeoutException&)
        {
            testRetryCount(2);
            retry2->opIdempotent(-1);
            testRetryCount(-1);
        }

        cout << "ok" << endl;
    }

    return retry1;
}
