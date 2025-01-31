// Copyright (c) ZeroC, Inc.

#include "CallbackI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

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
    throw CallbackException{3.14, "3.14"};
}

void
CallbackReceiverI::concurrentCallbackAsync(
    int number,
    function<void(int)> response,
    function<void(exception_ptr)> error,
    const Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        _callbacks.emplace_back(std::move(response), std::move(error), number);
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
    while (!_finishWaitCallback)
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

    while (_callback != expected)
    {
        _condVar.wait(lock);
    }
    _callback = 0;
}

void
CallbackReceiverI::waitCallbackOK()
{
    unique_lock<mutex> lock(_mutex);
    while (!_waitCallback)
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

    while (!_callbackWithPayload)
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

    while (_callbacks.size() != num)
    {
        _condVar.wait(lock);
    }

    for (const auto& p : _callbacks)
    {
        get<0>(p)(get<2>(p));
    }
    _callbacks.clear();
}

void
CallbackI::initiateCallbackAsync(
    optional<CallbackReceiverPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    if (proxy->ice_isTwoway())
    {
        proxy->callbackAsync(std::move(response), std::move(error), nullptr, current.ctx);
    }
    else
    {
        proxy->callback(current.ctx);
        response();
    }
}

void
CallbackI::initiateCallbackExAsync(
    optional<CallbackReceiverPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    if (proxy->ice_isTwoway())
    {
        proxy->callbackExAsync(std::move(response), std::move(error), nullptr, current.ctx);
    }
    else
    {
        proxy->callbackEx(current.ctx);
        response();
    }
}

void
CallbackI::initiateConcurrentCallbackAsync(
    int number,
    optional<CallbackReceiverPrx> proxy,
    function<void(int)> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    proxy->concurrentCallbackAsync(number, std::move(response), std::move(error), nullptr, current.ctx);
}

void
CallbackI::initiateWaitCallbackAsync(
    optional<CallbackReceiverPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    proxy->waitCallbackAsync(std::move(response), std::move(error), nullptr, current.ctx);
}

void
CallbackI::initiateCallbackWithPayloadAsync(
    optional<CallbackReceiverPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    Ice::ByteSeq seq(1000 * 1024, byte{0});
    proxy->callbackWithPayloadAsync(seq, std::move(response), std::move(error), nullptr, current.ctx);
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
