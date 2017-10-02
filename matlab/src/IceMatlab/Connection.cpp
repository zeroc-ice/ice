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
#include "Endpoint.h"
#include "Future.h"
#include "Util.h"

#define SELF (*(reinterpret_cast<shared_ptr<Ice::Connection>*>(self)))

using namespace std;
using namespace IceMatlab;

namespace
{

enum Field
{
    Type = 0,
    Underlying,
    Incoming,
    AdapterName,
    ConnectionId,
    LocalAddress,
    LocalPort,
    RemoteAddress,
    RemotePort,
    RcvSize,
    SndSize,
    McastAddress,
    McastPort,
    Headers,
    NumFields // Number of fields in structure, must be last
};

static const char* infoFields[] =
{
    "type",
    "underlying",
    "incoming",
    "adapterName",
    "connectionId",
    "localAddress",
    "localPort",
    "remoteAddress",
    "remotePort",
    "rcvSize",
    "sndSize",
    "mcastAddress",
    "mcastPort",
    "headers"
};

mxArray*
createInfo(shared_ptr<Ice::ConnectionInfo> info)
{
    //
    // Create and return a struct array containing the fields that describe the EndpointInfo object.
    // Some fields will be left unused.
    //

    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, Field::NumFields, infoFields);
    mxSetFieldByNumber(r, 0, Field::Incoming, createBool(info->incoming));
    mxSetFieldByNumber(r, 0, Field::AdapterName, createStringFromUTF8(info->adapterName));
    mxSetFieldByNumber(r, 0, Field::ConnectionId, createStringFromUTF8(info->connectionId));

    if(info->underlying)
    {
        auto u = createInfo(info->underlying);
        mxSetFieldByNumber(r, 0, Field::Underlying, u);
    }

    string type = "other";

    shared_ptr<Ice::IPConnectionInfo> ipInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(info);
    if(ipInfo)
    {
        type = "ip";
        mxSetFieldByNumber(r, 0, Field::LocalAddress, createStringFromUTF8(ipInfo->localAddress));
        mxSetFieldByNumber(r, 0, Field::LocalPort, createInt(ipInfo->localPort));
        mxSetFieldByNumber(r, 0, Field::RemoteAddress, createStringFromUTF8(ipInfo->remoteAddress));
        mxSetFieldByNumber(r, 0, Field::RemotePort, createInt(ipInfo->remotePort));
    }

    shared_ptr<Ice::UDPConnectionInfo> udpInfo = dynamic_pointer_cast<Ice::UDPConnectionInfo>(info);
    if(udpInfo)
    {
        type = "udp";
        mxSetFieldByNumber(r, 0, Field::McastAddress, createStringFromUTF8(udpInfo->mcastAddress));
        mxSetFieldByNumber(r, 0, Field::McastPort, createInt(udpInfo->mcastPort));
        mxSetFieldByNumber(r, 0, Field::RcvSize, createInt(udpInfo->rcvSize));
        mxSetFieldByNumber(r, 0, Field::SndSize, createInt(udpInfo->sndSize));
    }

    shared_ptr<Ice::TCPConnectionInfo> tcpInfo = dynamic_pointer_cast<Ice::TCPConnectionInfo>(info);
    if(tcpInfo)
    {
        type = "tcp";
        mxSetFieldByNumber(r, 0, Field::RcvSize, createInt(tcpInfo->rcvSize));
        mxSetFieldByNumber(r, 0, Field::SndSize, createInt(tcpInfo->sndSize));
    }

    shared_ptr<Ice::WSConnectionInfo> wsInfo = dynamic_pointer_cast<Ice::WSConnectionInfo>(info);
    if(wsInfo)
    {
        type = "ws";
        mxSetFieldByNumber(r, 0, Field::Headers, createStringMap(wsInfo->headers));
    }

    /* TODO: If we link with IceSSL
    if(dynamic_pointer_cast<IceSSL::ConnectionInfo>(info))
    {
        type = "ssl";
    }
    */

    mxSetFieldByNumber(r, 0, Field::Type, createStringFromUTF8(type));

    return r;
}

}

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
Ice_Connection_getEndpoint(void* self, void** endpoint)
{
    try
    {
        *endpoint = createEndpoint(SELF->getEndpoint());
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
Ice_Connection_timeout(void* self)
{
    try
    {
        return createResultValue(createInt(SELF->timeout()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
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
Ice_Connection_getInfo(void* self)
{
    try
    {
        shared_ptr<Ice::ConnectionInfo> info = SELF->getInfo();
        return createResultValue(createInfo(info));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
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
