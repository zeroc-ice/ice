// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include "ice.h"
#include "Future.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

void
IceMatlab::Future::token(function<void()> t)
{
    lock_guard<mutex> lock(_mutex);
    if(stateImpl() != State::Finished)
    {
        _token = std::move(t);
    }
}

bool
IceMatlab::Future::waitForState(State state, double timeout)
{
    unique_lock<mutex> lock(_mutex);
    if(timeout < 0)
    {
        _cond.wait(lock, [this, state]{ return state == this->stateImpl(); });
        return !_exception;
    }
    else
    {
        auto now = chrono::system_clock::now();
        auto stop = now + chrono::milliseconds(static_cast<long long>(timeout * 1000));
        bool b = _cond.wait_until(lock, stop, [this, state]{ return state == this->stateImpl(); });
        return b && !_exception;
    }
}

bool
IceMatlab::Future::waitForState(const string& s, double timeout)
{
    State state;
    if(s == "running")
    {
        state = State::Running;
    }
    else if(s == "sent")
    {
        state = State::Sent;
    }
    else if(s == "finished")
    {
        state = State::Finished;
    }
    else
    {
        throw std::invalid_argument("state must be one of 'running', 'sent' or 'finished'");
    }
    return waitForState(state, timeout);
}

void
IceMatlab::Future::exception(exception_ptr e)
{
    lock_guard<mutex> lock(_mutex);
    _token = nullptr;
    _exception = e;
    _cond.notify_all();
}

exception_ptr
IceMatlab::Future::getException() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    return _exception;
}

void
IceMatlab::Future::sent()
{
}

string
IceMatlab::Future::state() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    string st;
    switch(stateImpl())
    {
        case State::Running:
            st = "running";
            break;
        case State::Sent:
            st = "sent";
            break;
        case State::Finished:
            st = "finished";
            break;
    }
    return st;
}

void
IceMatlab::Future::cancel()
{
    lock_guard<mutex> lock(_mutex);
    if(_token)
    {
        _token();
        _token = nullptr;
    }
}

//
// SimpleFuture
//
IceMatlab::SimpleFuture::SimpleFuture() :
    _state(State::Running)
{
}

void
IceMatlab::SimpleFuture::done()
{
    lock_guard<mutex> lock(_mutex);
    _state = State::Finished;
    _cond.notify_all();
}

IceMatlab::Future::State
IceMatlab::SimpleFuture::stateImpl() const
{
    if(_exception)
    {
        return State::Finished;
    }
    else
    {
        return _state;
    }
}

extern "C"
{

mxArray*
Ice_SimpleFuture_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<SimpleFuture>*>(self);
    return 0;
}

mxArray*
Ice_SimpleFuture_wait(void* self)
{
    bool b = deref<SimpleFuture>(self)->waitForState("finished", -1);
    return createResultValue(createBool(b));
}

mxArray*
Ice_SimpleFuture_waitState(void* self, mxArray* st, double timeout)
{
    try
    {
        string s = getStringFromUTF16(st);
        bool b = deref<SimpleFuture>(self)->waitForState(s, timeout);
        return createResultValue(createBool(b));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_SimpleFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(deref<SimpleFuture>(self)->state()));
}

mxArray*
Ice_SimpleFuture_cancel(void* self)
{
    deref<SimpleFuture>(self)->cancel();
    return 0;
}

mxArray*
Ice_SimpleFuture_check(void* self)
{
    auto f = deref<SimpleFuture>(self);
    if(!f->waitForState(Future::State::Finished, -1))
    {
        assert(f->getException());
        try
        {
            rethrow_exception(f->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete reinterpret_cast<shared_ptr<SimpleFuture>*>(self);
            return convertException(ex);
        }
    }

    //
    // The C++ object won't be used after this.
    //
    delete reinterpret_cast<shared_ptr<SimpleFuture>*>(self);

    return 0;
}

}
