// Copyright (c) ZeroC, Inc.

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
        InfoType,
        Secure,
        Underlying,
        Timeout,
        Compress,
        Host,
        Port,
        SourceAddress,
        McastInterface,
        McastTtl,
        Resource,
        RawEncoding,
        RawBytes,
        NumFields // Number of fields in structure, must be last
    };

    static const char* infoFields[] = {
        "type",
        "infoType",
        "secure",
        "underlying",
        "timeout",
        "compress",
        "host",
        "port",
        "sourceAddress",
        "mcastInterface",
        "mcastTtl",
        "resource",
        "rawEncoding",
        "rawBytes"};

    mxArray* createInfo(const shared_ptr<Ice::EndpointInfo>& info)
    {
        // Create and return a struct array containing the fields that describe the EndpointInfo object.
        // Some fields will be left unused.

        mwSize dims[2] = {1, 1};
        auto r = mxCreateStructArray(2, dims, Field::NumFields, infoFields);
        mxSetFieldByNumber(r, 0, Field::Type, createInt(info->type()));
        mxSetFieldByNumber(r, 0, Field::Secure, createBool(info->secure()));
        mxSetFieldByNumber(r, 0, Field::Timeout, createInt(info->timeout));
        mxSetFieldByNumber(r, 0, Field::Compress, createBool(info->compress));

        // Don't use info->type() to determine the type of the EndpointInfo object. When an endpoint is the
        // underlying endpoint of a parent, the child's value for type() is the same as its parent. We have
        // to use type casts instead.

        if (info->underlying)
        {
            auto u = createInfo(info->underlying);
            mxSetFieldByNumber(r, 0, Field::Underlying, u);

            auto wsInfo = dynamic_pointer_cast<Ice::WSEndpointInfo>(info);
            if (wsInfo)
            {
                mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::WSEndpointType));
                mxSetFieldByNumber(r, 0, Field::Resource, createStringFromUTF8(wsInfo->resource));
            }
            else if (dynamic_pointer_cast<Ice::SSL::EndpointInfo>(info))
            {
                mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::SSLEndpointType));
            }
        }
        else
        {
            auto ipInfo = dynamic_pointer_cast<Ice::IPEndpointInfo>(info);
            if (ipInfo)
            {
                mxSetFieldByNumber(r, 0, Field::Host, createStringFromUTF8(ipInfo->host));
                mxSetFieldByNumber(r, 0, Field::Port, createInt(ipInfo->port));
                mxSetFieldByNumber(r, 0, Field::SourceAddress, createStringFromUTF8(ipInfo->sourceAddress));

                auto udpInfo = dynamic_pointer_cast<Ice::UDPEndpointInfo>(info);
                if (udpInfo)
                {
                    mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::UDPEndpointType));
                    mxSetFieldByNumber(r, 0, Field::McastInterface, createStringFromUTF8(udpInfo->mcastInterface));
                    mxSetFieldByNumber(r, 0, Field::McastTtl, createInt(udpInfo->mcastTtl));
                }
                else if (dynamic_pointer_cast<Ice::TCPEndpointInfo>(info))
                {
                    mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::TCPEndpointType));
                }
            }
            else
            {
                auto opaqueInfo = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(info);
                if (opaqueInfo)
                {
                    // We don't set InfoType because there is no reserved int16 constant for Opaque.
                    mxSetFieldByNumber(r, 0, Field::RawEncoding, createEncodingVersion(opaqueInfo->rawEncoding));
                    const std::byte* p = &opaqueInfo->rawBytes[0];
                    mxSetFieldByNumber(r, 0, Field::RawBytes, createByteArray(p, p + opaqueInfo->rawBytes.size()));
                }
            }
        }

        return r;
    }
}

extern "C"
{
    mxArray* Ice_Endpoint_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<Ice::Endpoint>*>(self);
        return 0;
    }

    mxArray* Ice_Endpoint_equals(void* self, void* other)
    {
        assert(other); // Wrapper only calls this function for non-nil arguments.
        try
        {
            return createResultValue(
                createBool(Ice::targetEqualTo(deref<Ice::Endpoint>(self), deref<Ice::Endpoint>(other))));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Endpoint_toString(void* self)
    {
        try
        {
            return createResultValue(createStringFromUTF8(deref<Ice::Endpoint>(self)->toString()));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Endpoint_getInfo(void* self)
    {
        try
        {
            shared_ptr<Ice::EndpointInfo> info = deref<Ice::Endpoint>(self)->getInfo();
            return createResultValue(createInfo(info));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }
}
