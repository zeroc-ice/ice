// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <InstrumentationI.h>
#include <SystemFailure.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class CallbackSuccess : public IceUtil::Shared, public CallbackBase
{
public:

    void response()
    {
        called();
    }

    void exception(const ::Ice::Exception&)
    {
        test(false);
    }
};
typedef IceUtil::Handle<CallbackSuccess> CallbackSuccessPtr;

class CallbackFail : public IceUtil::Shared, public CallbackBase
{
public:

    void response()
    {
        test(false);
    }

    void exception(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::ConnectionLostException*>(&ex) ||
             dynamic_cast<const Ice::UnknownLocalException*>(&ex));
        called();
    }
};
typedef IceUtil::Handle<CallbackFail> CallbackFailPtr;

RetryPrx
allTests(const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2, const string& ref)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base1 = communicator->stringToProxy(ref);
    test(base1);
    Ice::ObjectPrx base2 = communicator->stringToProxy(ref);
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    RetryPrx retry1 = RetryPrx::checkedCast(base1);
    test(retry1);
    test(retry1 == base1);
    RetryPrx retry2 = RetryPrx::checkedCast(base2);
    test(retry2);
    test(retry2 == base2);
    cout << "ok" << endl;

    cout << "calling regular operation with first proxy... " << flush;
    retry1->op(false);
    cout << "ok" << endl;

    testInvocationCount(3);

    cout << "calling operation to kill connection with second proxy... " << flush;
    try
    {
        retry2->op(true);
        test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
        // Expected with collocation
    }
    catch(const Ice::ConnectionLostException&)
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

    CallbackSuccessPtr cb1 = new CallbackSuccess();
    CallbackFailPtr cb2 = new CallbackFail();

    cout << "calling regular AMI operation with first proxy... " << flush;
    retry1->begin_op(false, newCallback_Retry_op(cb1, &CallbackSuccess::response, &CallbackSuccess::exception));
    cb1->check();
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling AMI operation to kill connection with second proxy... " << flush;
    retry2->begin_op(true, newCallback_Retry_op(cb2, &CallbackFail::response, &CallbackFail::exception));
    cb2->check();
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling regular AMI operation with first proxy again... " << flush;
    retry1->begin_op(false, newCallback_Retry_op(cb1, &CallbackSuccess::response, &CallbackSuccess::exception));
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
    test(retry1->end_opIdempotent(retry1->begin_opIdempotent(4)) == 4);
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(4);
    cout << "ok" << endl;

    cout << "testing non-idempotent operation... " << flush;
    try
    {
        retry1->opNotIdempotent();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    try
    {
        retry1->end_opNotIdempotent(retry1->begin_opNotIdempotent());
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    if(!retry1->ice_getConnection())
    {
        testInvocationCount(-1);
        cout << "testing system exception... " << flush;
        try
        {
            retry1->opSystemException();
            test(false);
        }
        catch(const SystemFailure&)
        {
        }
        testInvocationCount(1);
        testFailureCount(1);
        testRetryCount(0);
        try
        {
            retry1->end_opSystemException(retry1->begin_opSystemException());
            test(false);
        }
        catch(const SystemFailure&)
        {
        }
        testInvocationCount(1);
        testFailureCount(1);
        testRetryCount(0);
        cout << "ok" << endl;
    }

    cout << "testing invocation timeout and retries... " << flush;
    retry2 = RetryPrx::checkedCast(communicator2->stringToProxy(retry1->ice_toString()));
    try
    {
        retry2->ice_invocationTimeout(500)->opIdempotent(4);  // No more than 2 retries before timeout kicks-in
        test(false);
    }
    catch(const Ice::InvocationTimeoutException&)
    {
        testRetryCount(2);
        retry2->opIdempotent(-1); // Reset the counter
        testRetryCount(-1);
    }
    try
    {
        // No more than 2 retries before timeout kicks-in
        RetryPrx prx = retry2->ice_invocationTimeout(500);
        prx->end_opIdempotent(prx->begin_opIdempotent(4));
        test(false);
    }
    catch(const Ice::InvocationTimeoutException&)
    {
        testRetryCount(2);
        retry2->opIdempotent(-1);
        testRetryCount(-1);
    }
    cout << "ok" << endl;

    return retry1;
}
