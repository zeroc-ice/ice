// Copyright (c) ZeroC, Inc.

#include "../../cpp/src/Ice/SSL/SSLUtil.h"
#include "Future.h"
#include "Ice/Ice.h"
#include "Util.h"
#include "ice.h"

using namespace std;
using namespace IceMatlab;

namespace
{
    enum Field
    {
        Type = 0,
        Underlying,
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
        PeerCertificate,
        NumFields // Number of fields in structure, must be last
    };

    static const char* infoFields[] = {
        "type",
        "underlying",
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
        "peerCertificate"};

    mxArray* createInfo(const shared_ptr<Ice::ConnectionInfo>& info)
    {
        // Create and return a struct array containing the fields that describe the EndpointInfo object.
        // Some fields will be left unused.

        mwSize dims[2] = {1, 1};
        auto r = mxCreateStructArray(2, dims, Field::NumFields, infoFields);
        mxSetFieldByNumber(r, 0, Field::ConnectionId, createStringFromUTF8(info->connectionId));

        string type = "other";

        if (info->underlying)
        {
            auto u = createInfo(info->underlying);
            mxSetFieldByNumber(r, 0, Field::Underlying, u);

            auto sslInfo = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(info);
            if (sslInfo)
            {
                type = "ssl";
                string encoded;
                if (sslInfo->peerCertificate)
                {
                    encoded = Ice::SSL::encodeCertificate(sslInfo->peerCertificate);
                }
                mxSetFieldByNumber(r, 0, Field::PeerCertificate, createStringFromUTF8(encoded));
            }
            else
            {
                auto wsInfo = dynamic_pointer_cast<Ice::WSConnectionInfo>(info);
                if (wsInfo)
                {
                    type = "ws";
                    mxSetFieldByNumber(r, 0, Field::Headers, createStringMap(wsInfo->headers));
                }
            }
        }
        else
        {
            auto ipInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(info);
            if (ipInfo)
            {
                mxSetFieldByNumber(r, 0, Field::LocalAddress, createStringFromUTF8(ipInfo->localAddress));
                mxSetFieldByNumber(r, 0, Field::LocalPort, createInt(ipInfo->localPort));
                mxSetFieldByNumber(r, 0, Field::RemoteAddress, createStringFromUTF8(ipInfo->remoteAddress));
                mxSetFieldByNumber(r, 0, Field::RemotePort, createInt(ipInfo->remotePort));

                auto tcpInfo = dynamic_pointer_cast<Ice::TCPConnectionInfo>(info);
                if (tcpInfo)
                {
                    type = "tcp";
                    mxSetFieldByNumber(r, 0, Field::RcvSize, createInt(tcpInfo->rcvSize));
                    mxSetFieldByNumber(r, 0, Field::SndSize, createInt(tcpInfo->sndSize));
                }
                else
                {
                    auto udpInfo = dynamic_pointer_cast<Ice::UDPConnectionInfo>(info);
                    if (udpInfo)
                    {
                        type = "udp";
                        mxSetFieldByNumber(r, 0, Field::McastAddress, createStringFromUTF8(udpInfo->mcastAddress));
                        mxSetFieldByNumber(r, 0, Field::McastPort, createInt(udpInfo->mcastPort));
                        mxSetFieldByNumber(r, 0, Field::RcvSize, createInt(udpInfo->rcvSize));
                        mxSetFieldByNumber(r, 0, Field::SndSize, createInt(udpInfo->sndSize));
                    }
                }
            }
        }

        mxSetFieldByNumber(r, 0, Field::Type, createStringFromUTF8(type));
        return r;
    }
}

extern "C"
{
    mxArray* Ice_Connection_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<Ice::Connection>*>(self);
        return createEmptyArray();
    }

    mxArray* Ice_Connection_equals(void* self, void* other)
    {
        assert(other); // Wrapper only calls this function for non-nil arguments.
        try
        {
            return createResultValue(createBool(deref<Ice::Connection>(self) == deref<Ice::Connection>(other)));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Connection_abort(void* self)
    {
        deref<Ice::Connection>(self)->abort();
        return createEmptyArray();
    }

    mxArray* Ice_Connection_close(void* self, void** future)
    {
        auto connection = deref<Ice::Connection>(self);

        auto futurePtr = make_shared<SimpleFuture>();
        connection->close(
            [futurePtr]() { futurePtr->done(); },
            [futurePtr](exception_ptr closeException) { futurePtr->exception(closeException); });

        *future = new shared_ptr<SimpleFuture>(move(futurePtr));
        return createEmptyArray();
    }

    mxArray* Ice_Connection_createProxy(void* self, mxArray* id, void** r)
    {
        try
        {
            Ice::Identity ident;
            getIdentity(id, ident);
            Ice::ObjectPrx proxy = deref<Ice::Connection>(self)->createProxy(ident);
            *r = createProxy(std::move(proxy));
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_Connection_flushBatchRequests(void* self, mxArray* c)
    {
        try
        {
            auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
            deref<Ice::Connection>(self)->flushBatchRequests(mode);
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_Connection_flushBatchRequestsAsync(void* self, mxArray* c, void** future)
    {
        *future = 0;
        auto f = make_shared<SimpleFuture>();

        try
        {
            auto mode = static_cast<Ice::CompressBatch>(getEnumerator(c, "Ice.CompressBatch"));
            function<void()> token = deref<Ice::Connection>(self)->flushBatchRequestsAsync(
                mode,
                [f](exception_ptr e) { f->exception(e); },
                [f](bool /*sentSynchronously*/) { f->done(); });
            f->token(token);
            *future = new shared_ptr<SimpleFuture>(move(f));
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_Connection_getEndpoint(void* self, void** endpoint)
    {
        try
        {
            *endpoint = createShared<Ice::Endpoint>(deref<Ice::Connection>(self)->getEndpoint());
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_Connection_type(void* self)
    {
        try
        {
            return createResultValue(createStringFromUTF8(deref<Ice::Connection>(self)->type()));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Connection_toString(void* self)
    {
        try
        {
            return createResultValue(createStringFromUTF8(deref<Ice::Connection>(self)->toString()));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Connection_getInfo(void* self)
    {
        try
        {
            shared_ptr<Ice::ConnectionInfo> info = deref<Ice::Connection>(self)->getInfo();
            return createResultValue(createInfo(info));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Connection_setBufferSize(void* self, int rcvSize, int sndSize)
    {
        try
        {
            deref<Ice::Connection>(self)->setBufferSize(rcvSize, sndSize);
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_Connection_throwException(void* self)
    {
        try
        {
            deref<Ice::Connection>(self)->throwException();
            return createEmptyArray();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }
}
