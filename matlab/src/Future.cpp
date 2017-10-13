// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    if(!isFinished())
    {
        _token = std::move(t);
    }
}

bool
IceMatlab::Future::waitUntilFinished()
{
    unique_lock<mutex> lock(_mutex);
    _cond.wait(lock, [this]{ return this->isFinished(); });
    return !_exception;
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
    _done(false)
{
}

void
IceMatlab::SimpleFuture::done()
{
    lock_guard<mutex> lock(_mutex);
    _done = true;
    _cond.notify_all();
}

string
IceMatlab::SimpleFuture::state() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    if(_exception || _done)
    {
        return "finished";
    }
    else
    {
        return "running";
    }
}

bool
IceMatlab::SimpleFuture::isFinished() const
{
    return _done || _exception;
}

#define SFSELF (*(reinterpret_cast<shared_ptr<SimpleFuture>*>(self)))

extern "C"
{

mxArray*
Ice_SimpleFuture_unref(void* self)
{
    delete &SFSELF;
    return 0;
}

mxArray*
Ice_SimpleFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = SFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

mxArray*
Ice_SimpleFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(SFSELF->state()));
}

mxArray*
Ice_SimpleFuture_cancel(void* self)
{
    SFSELF->cancel();
    return 0;
}

mxArray*
Ice_SimpleFuture_check(void* self)
{
    if(!SFSELF->waitUntilFinished())
    {
        assert(SFSELF->getException());
        try
        {
            rethrow_exception(SFSELF->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete &SFSELF;
            return convertException(ex);
        }
    }

    //
    // The C++ object won't be used after this.
    //
    delete &SFSELF;

    return 0;
}

}
