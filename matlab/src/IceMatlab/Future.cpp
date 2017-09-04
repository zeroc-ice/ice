// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include "icematlab.h"
#include "Future.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

void
IceMatlab::Future::token(function<void()> t)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(!isFinished())
    {
        _token = std::move(t);
    }
}

bool
IceMatlab::Future::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(!isFinished())
    {
        wait();
    }
    return !_exception;
}

void
IceMatlab::Future::exception(exception_ptr e)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _token = nullptr;
    _exception = e;
    notifyAll();
}

exception_ptr
IceMatlab::Future::getException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _exception;
}

void
IceMatlab::Future::sent()
{
}

void
IceMatlab::Future::cancel()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_token)
    {
        _token();
        _token = nullptr;
    }
}

IceMatlab::SentFuture::SentFuture() :
    _sent(false)
{
}

void
IceMatlab::SentFuture::sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _sent = true;
    notifyAll();
}

string
IceMatlab::SentFuture::state() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception || _sent)
    {
        return "finished";
    }
    else
    {
        return "running";
    }
}

bool
IceMatlab::SentFuture::isFinished() const
{
    return _sent || _exception;
}

#define SFSELF (*(reinterpret_cast<shared_ptr<SentFuture>*>(self)))

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_SentFuture__release(void* self)
{
    delete &SFSELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_SentFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_SentFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = SFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_SentFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(SFSELF->state()));
}

EXPORTED_FUNCTION mxArray*
Ice_SentFuture_cancel(void* self)
{
    SFSELF->cancel();
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_SentFuture_check(void* self)
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
            return convertException(ex);
        }
    }

    return 0;
}

}
