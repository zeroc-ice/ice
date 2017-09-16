// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Ice.h>
#include "icematlab.h"
#include "Future.h"
#include "Util.h"

#define SELF (*(reinterpret_cast<shared_ptr<Ice::Connection>*>(self)))

using namespace std;
using namespace IceMatlab;

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_Connection__release(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_close(void* self, mxArray* m)
{
    try
    {
        auto mode = static_cast<Ice::ConnectionClose>(getEnumerator(m, "Ice.ConnectionClose"));
        SELF->close(mode);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_closeAsync(void* self, mxArray* m, void** future)
{
    *future = 0;
    auto c = SELF;
    auto f = make_shared<SimpleFuture>();

    thread t([m, c, f]
        {
            try
            {
                auto mode = static_cast<Ice::ConnectionClose>(getEnumerator(m, "Ice.ConnectionClose"));
                c->close(mode);
                f->done();
            }
            catch(const std::exception&)
            {
                f->exception(current_exception());
            }
        });
    t.detach();
    *future = new shared_ptr<SimpleFuture>(f);
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_createProxy(void* self, mxArray* id, void** r)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        shared_ptr<Ice::ObjectPrx> proxy = SELF->createProxy(ident);
        *r = new shared_ptr<Ice::ObjectPrx>(proxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_flushBatchRequests(void* self, mxArray* c)
{
    try
    {
        auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
        SELF->flushBatchRequests(mode);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_flushBatchRequestsAsync(void* self, mxArray* c, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
        function<void()> token = SELF->flushBatchRequestsAsync(
            mode,
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->done();
            });
        f->token(token);
        *future = new shared_ptr<SimpleFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_heartbeat(void* self)
{
    try
    {
        SELF->heartbeat();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_heartbeatAsync(void* self, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        function<void()> token = SELF->heartbeatAsync(
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->done();
            });
        f->token(token);
        *future = new shared_ptr<SimpleFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_setACM(void* self, mxArray* t, mxArray* c, mxArray* h)
{
    Ice::optional<int> timeout;
    Ice::optional<Ice::ACMClose> close;
    Ice::optional<Ice::ACMHeartbeat> heartbeat;

    try
    {
        if(!mxIsEmpty(t))
        {
            if(!mxIsScalar(t))
            {
                throw invalid_argument("scalar value required for timeout");
            }
            if(!mxIsNumeric(t))
            {
                throw invalid_argument("numeric value required for timeout");
            }
            timeout = static_cast<int>(mxGetScalar(t));
        }
        if(!mxIsEmpty(c))
        {
            close = static_cast<Ice::ACMClose>(getEnumerator(c, "Ice.ACMClose"));
        }
        if(!mxIsEmpty(h))
        {
            heartbeat = static_cast<Ice::ACMHeartbeat>(getEnumerator(h, "Ice.ACMHeartbeat"));
        }
        SELF->setACM(timeout, close, heartbeat);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_getACM(void* self)
{
    try
    {
        auto acm = SELF->getACM();
        mxArray* params[3];
        params[0] = createInt(acm.timeout);
        params[1] = createEnumerator("Ice.ACMClose", static_cast<int>(acm.close));
        params[2] = createEnumerator("Ice.ACMHeartbeat", static_cast<int>(acm.heartbeat));
        mxArray* r;
        mexCallMATLAB(1, &r, 3, params, "Ice.ACM");
        return createResultValue(r);
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_type(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->type()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_timeout(void* self, int* timeout)
{
    try
    {
        *timeout = SELF->timeout();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_toString(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->toString()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_setBufferSize(void* self, int rcvSize, int sndSize)
{
    try
    {
        SELF->setBufferSize(rcvSize, sndSize);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Connection_throwException(void* self)
{
    try
    {
        SELF->throwException();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
