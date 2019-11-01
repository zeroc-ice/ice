//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <CallbackI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
CallbackReceiverI::callback(int token, const Current&)
{
    {
        unique_lock<mutex> lock(_mutex);
        checkForHold(lock);
        if(token != _lastToken)
        {
            _callback = 0;
            _lastToken = token;
        }
        ++_callback;
    }
    _condVar.notify_all();
}

void
CallbackReceiverI::callbackWithPayload(Ice::ByteSeq, const Current&)
{
    {
        unique_lock<mutex> lock(_mutex);
        checkForHold(lock);
        ++_callbackWithPayload;
    }
    _condVar.notify_all();
}

int
CallbackReceiverI::callbackOK(int count, int token)
{
    unique_lock<mutex> lock(_mutex);

    while(_lastToken != token || _callback < count)
    {
        _condVar.wait(lock);
    }

    _callback -= count;
    return _callback;
}

int
CallbackReceiverI::callbackWithPayloadOK(int count)
{
    unique_lock<mutex> lock(_mutex);

    while(_callbackWithPayload < count)
    {
        _condVar.wait(lock);
    }

    _callbackWithPayload -= count;
    return _callbackWithPayload;
}

void
CallbackReceiverI::hold()
{
    lock_guard<mutex> lg(_mutex);
    _holding = true;
}

void
CallbackReceiverI::activate()
{
    {
        lock_guard<mutex> lg(_mutex);
        _holding = false;
    }
    _condVar.notify_all();
}

void
CallbackReceiverI::checkForHold(unique_lock<mutex>& lock)
{
    while(_holding)
    {
        _condVar.wait(lock);
    }
}

void
CallbackI::initiateCallbackAsync(shared_ptr<CallbackReceiverPrx> proxy, int token,
                                 function<void()> response, function<void(exception_ptr)> error,
                                 const Current& current)
{
    auto p = current.ctx.find("serverOvrd");
    auto ctx = current.ctx;
    if(p != current.ctx.end())
    {
        ctx["_ovrd"] = p->second;
    }

    if(proxy->ice_isTwoway())
    {
        proxy->callbackAsync(token, move(response), move(error), nullptr, ctx);
    }
    else
    {
        proxy->callback(token, ctx);
        response();
    }
}

void
CallbackI::initiateCallbackWithPayloadAsync(shared_ptr<CallbackReceiverPrx> proxy,
                                            function<void()> response, function<void(exception_ptr)> error,
                                            const Current& current)
{
    auto p = current.ctx.find("serverOvrd");
    auto ctx = current.ctx;
    if(p != current.ctx.end())
    {
        ctx["_ovrd"] = p->second;
    }

    Ice::ByteSeq seq(1000 * 1024, 0);
    if(proxy->ice_isTwoway())
    {
        proxy->callbackWithPayloadAsync(seq, move(response), move(error), nullptr, ctx);
    }
    else
    {
        proxy->callbackWithPayload(seq, ctx);
        response();
    }
}

void
CallbackI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
