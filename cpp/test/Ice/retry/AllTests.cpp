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

RetryPrxPtr
allTests(const Ice::CommunicatorPtr& communicator, const Ice::CommunicatorPtr& communicator2, const string& ref)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrxPtr base1 = communicator->stringToProxy(ref);
    test(base1);
    Ice::ObjectPrxPtr base2 = communicator->stringToProxy(ref);
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    RetryPrxPtr retry1 = ICE_CHECKED_CAST(RetryPrx, base1);
    test(retry1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(retry1, base1));
#else
    test(retry1 == base1);
#endif
    RetryPrxPtr retry2 = ICE_CHECKED_CAST(RetryPrx, base2);
    test(retry2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(retry2, base2));
#else
    test(retry2 == base2);
#endif
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
#ifdef ICE_CPP11_MAPPING
    retry1->opAsync(false,
        [cb1]()
        {
            cb1->response();
        },
        [cb1](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch(const Ice::Exception& ex)
            {
                cb1->exception(ex);
            }
        });
#else
    retry1->begin_op(false, newCallback_Retry_op(cb1, &CallbackSuccess::response, &CallbackSuccess::exception));
#endif
    cb1->check();
    testInvocationCount(1);
    testFailureCount(0);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling AMI operation to kill connection with second proxy... " << flush;
#ifdef ICE_CPP11_MAPPING
    retry2->opAsync(true,
        [cb2]()
        {
            cb2->response();
        },
        [cb2](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch(const Ice::Exception& ex)
            {
                cb2->exception(ex);
            }
        });
#else
    retry2->begin_op(true, newCallback_Retry_op(cb2, &CallbackFail::response, &CallbackFail::exception));
#endif
    cb2->check();
    testInvocationCount(1);
    testFailureCount(1);
    testRetryCount(0);
    cout << "ok" << endl;

    cout << "calling regular AMI operation with first proxy again... " << flush;
#ifdef ICE_CPP11_MAPPING
    retry1->opAsync(false,
        [cb1]()
        {
            cb1->response();
        },
        [cb1](exception_ptr err)
        {
            try
            {
                rethrow_exception(err);
            }
            catch(const Ice::Exception& ex)
            {
                cb1->exception(ex);
            }
        });
#else
    retry1->begin_op(false, newCallback_Retry_op(cb1, &CallbackSuccess::response, &CallbackSuccess::exception));
#endif
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
#ifdef ICE_CPP11_MAPPING
    test(retry1->opIdempotentAsync(4).get() == 4);
#else
    test(retry1->end_opIdempotent(retry1->begin_opIdempotent(4)) == 4);
#endif
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
#ifdef ICE_CPP11_MAPPING
        retry1->opNotIdempotentAsync().get();
#else
        retry1->end_opNotIdempotent(retry1->begin_opNotIdempotent());
#endif
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
#ifdef ICE_CPP11_MAPPING
            retry1->opSystemExceptionAsync().get();
#else
            retry1->end_opSystemException(retry1->begin_opSystemException());
#endif
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
    retry2 = ICE_CHECKED_CAST(RetryPrx, communicator2->stringToProxy(retry1->ice_toString()));
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
        RetryPrxPtr prx = retry2->ice_invocationTimeout(500);
#ifdef ICE_CPP11_MAPPING
        prx->opIdempotentAsync(4).get();
#else
        prx->end_opIdempotent(prx->begin_opIdempotent(4));
#endif
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
