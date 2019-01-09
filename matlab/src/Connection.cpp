// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>
#include "ice.h"
#include "Future.h"
#include "Util.h"

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
    Cipher,
    Certs,
    Verified,
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
    "headers",
    "cipher",
    "certs",
    "verified"
};

mxArray*
createInfo(const shared_ptr<Ice::ConnectionInfo>& info)
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

    shared_ptr<IceSSL::ConnectionInfo> sslInfo = dynamic_pointer_cast<IceSSL::ConnectionInfo>(info);
    if(sslInfo)
    {
        type = "ssl";
        mxSetFieldByNumber(r, 0, Field::Cipher, createStringFromUTF8(sslInfo->cipher));
        mxSetFieldByNumber(r, 0, Field::Certs, createCertificateList(sslInfo->certs));
        mxSetFieldByNumber(r, 0, Field::Verified, createBool(sslInfo->verified));
    }

    mxSetFieldByNumber(r, 0, Field::Type, createStringFromUTF8(type));

    return r;
}

}

extern "C"
{

mxArray*
Ice_Connection_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<Ice::Connection>*>(self);
    return 0;
}

mxArray*
Ice_Connection_equals(void* self, void* other)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        return createResultValue(createBool(deref<Ice::Connection>(self) == deref<Ice::Connection>(other)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_close(void* self, mxArray* m)
{
    try
    {
        auto mode = static_cast<Ice::ConnectionClose>(getEnumerator(m, "Ice.ConnectionClose"));
        deref<Ice::Connection>(self)->close(mode);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_closeAsync(void* self, mxArray* m, void** future)
{
    *future = 0;
    auto c = deref<Ice::Connection>(self);
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
    *future = new shared_ptr<SimpleFuture>(move(f));
    return 0;
}

mxArray*
Ice_Connection_createProxy(void* self, mxArray* id, void** r)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        auto proxy = deref<Ice::Connection>(self)->createProxy(ident);
        *r = new shared_ptr<Ice::ObjectPrx>(move(proxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_flushBatchRequests(void* self, mxArray* c)
{
    try
    {
        auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
        deref<Ice::Connection>(self)->flushBatchRequests(mode);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_flushBatchRequestsAsync(void* self, mxArray* c, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
        function<void()> token = deref<Ice::Connection>(self)->flushBatchRequestsAsync(
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
        *future = new shared_ptr<SimpleFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_getEndpoint(void* self, void** endpoint)
{
    try
    {
        *endpoint = createShared<Ice::Endpoint>(deref<Ice::Connection>(self)->getEndpoint());
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_heartbeat(void* self)
{
    try
    {
        deref<Ice::Connection>(self)->heartbeat();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_heartbeatAsync(void* self, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        function<void()> token = deref<Ice::Connection>(self)->heartbeatAsync(
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->done();
            });
        f->token(token);
        *future = new shared_ptr<SimpleFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
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
        deref<Ice::Connection>(self)->setACM(timeout, close, heartbeat);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_getACM(void* self)
{
    try
    {
        auto acm = deref<Ice::Connection>(self)->getACM();
        mxArray* params[3];
        params[0] = createInt(acm.timeout);
        params[1] = createInt(static_cast<int>(acm.close)); // The integer is converted to the enumerator.
        params[2] = createInt(static_cast<int>(acm.heartbeat)); // The integer is converted to the enumerator.
        mxArray* r;
        mexCallMATLAB(1, &r, 3, params, "Ice.ACM");
        return createResultValue(r);
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_type(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::Connection>(self)->type()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_timeout(void* self)
{
    try
    {
        return createResultValue(createInt(deref<Ice::Connection>(self)->timeout()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_toString(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::Connection>(self)->toString()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_getInfo(void* self)
{
    try
    {
        shared_ptr<Ice::ConnectionInfo> info = deref<Ice::Connection>(self)->getInfo();
        return createResultValue(createInfo(info));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Connection_setBufferSize(void* self, int rcvSize, int sndSize)
{
    try
    {
        deref<Ice::Connection>(self)->setBufferSize(rcvSize, sndSize);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Connection_throwException(void* self)
{
    try
    {
        deref<Ice::Connection>(self)->throwException();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
