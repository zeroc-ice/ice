//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <CallbackI.h>
#include <TestHelper.h>

using namespace std;
using namespace std::chrono_literals;

using namespace Ice;
using namespace Test;

void
CallbackReceiverI::callback(const Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        ++_callback;
    }
    _condVar.notify_all();
}

void
CallbackReceiverI::callbackEx(const Current& current)
{
    callback(current);
    CallbackException ex;
    ex.someValue = 3.14;
    ex.someString = "3.14";
    throw ex;
}

void
CallbackReceiverI::concurrentCallbackAsync(int number, function<void(int)> response,
                                           function<void(exception_ptr)> error,
                                           const Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        _callbacks.emplace_back(move(response), move(error), number);
    }
    _condVar.notify_all();
}

void
CallbackReceiverI::waitCallback(const Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        assert(!_waitCallback);
        _waitCallback = true;
    }
    _condVar.notify_all();

    unique_lock<mutex> lock(_mutex);
    while(!_finishWaitCallback)
    {
        _condVar.wait(lock);
    }
    _finishWaitCallback = false;
}

void
CallbackReceiverI::callbackWithPayload(Ice::ByteSeq, const Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        assert(!_callbackWithPayload);
        _callbackWithPayload = true;
    }
    _condVar.notify_all();
}

void
CallbackReceiverI::callbackOK(int expected)
{
    unique_lock<mutex> lock(_mutex);

    while(_callback != expected)
    {
        _condVar.wait(lock);
    }
    _callback = 0;
}

void
CallbackReceiverI::waitCallbackOK()
{
    unique_lock<mutex> lock(_mutex);
    while(!_waitCallback)
    {
        _condVar.wait_for(lock, 30s);
        test(_waitCallback);
    }

    _waitCallback = false;
}

void
CallbackReceiverI::callbackWithPayloadOK()
{
    unique_lock<mutex> lock(_mutex);

    while(!_callbackWithPayload)
    {
        _condVar.wait(lock);
    }

    _callbackWithPayload = false;
}

void
CallbackReceiverI::notifyWaitCallback()
{
    {
        lock_guard<mutex> lg(_mutex);
        _finishWaitCallback = true;
    }
    _condVar.notify_all();
};

void
CallbackReceiverI::answerConcurrentCallbacks(unsigned int num)
{
    unique_lock<mutex> lock(_mutex);

    while(_callbacks.size() != num)
    {
        _condVar.wait(lock);
    }

    for(const auto& p : _callbacks)
    {
        get<0>(p)(get<2>(p));
    }
    _callbacks.clear();
}

void
CallbackI::initiateCallbackAsync(shared_ptr<CallbackReceiverPrx> proxy,
                                 function<void()> response,
                                 function<void(exception_ptr)> error,
                                 const Current& current)
{
    if(proxy->ice_isTwoway())
    {
        proxy->callbackAsync(move(response), move(error), nullptr, current.ctx);
    }
    else
    {
        proxy->callback(current.ctx);
        response();
    }
}

void
CallbackI::initiateCallbackExAsync(shared_ptr<CallbackReceiverPrx> proxy,
                                   function<void()> response,
                                   function<void(exception_ptr)> error,
                                   const Current& current)
{
    if(proxy->ice_isTwoway())
    {
        proxy->callbackExAsync(move(response), move(error), nullptr, current.ctx);
    }
    else
    {
        proxy->callbackEx(current.ctx);
        response();
    }
}

void
CallbackI::initiateConcurrentCallbackAsync(int number, shared_ptr<CallbackReceiverPrx> proxy,
                                           function<void(int)> response,
                                           function<void(exception_ptr)> error,
                                           const Current& current)
{
    proxy->concurrentCallbackAsync(number, move(response), move(error), nullptr, current.ctx);
}

void
CallbackI::initiateWaitCallbackAsync(shared_ptr<CallbackReceiverPrx> proxy,
                                   function<void()> response,
                                   function<void(exception_ptr)> error,
                                   const Current& current)
{
    proxy->waitCallbackAsync(move(response), move(error), nullptr, current.ctx);
}

void
CallbackI::initiateCallbackWithPayloadAsync(shared_ptr<CallbackReceiverPrx> proxy,
                                            function<void()> response,
                                            function<void(exception_ptr)> error,
                                            const Current& current)
{
    Ice::ByteSeq seq(1000 * 1024, 0);
    proxy->callbackWithPayloadAsync(seq, move(response), move(error), nullptr, current.ctx);
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
