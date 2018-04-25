// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>
#include "ice.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

namespace
{

enum Field
{
    Type = 0,
    InfoType,
    Datagram,
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

static const char* infoFields[] =
{
    "type",
    "infoType",
    "datagram",
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
    "rawBytes"
};

mxArray*
createInfo(const shared_ptr<Ice::EndpointInfo>& info)
{
    //
    // Create and return a struct array containing the fields that describe the EndpointInfo object.
    // Some fields will be left unused.
    //

    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, Field::NumFields, infoFields);
    mxSetFieldByNumber(r, 0, Field::Type, createInt(info->type()));
    mxSetFieldByNumber(r, 0, Field::Datagram, createBool(info->datagram()));
    mxSetFieldByNumber(r, 0, Field::Secure, createBool(info->secure()));
    mxSetFieldByNumber(r, 0, Field::Timeout, createInt(info->timeout));
    mxSetFieldByNumber(r, 0, Field::Compress, createBool(info->compress));

    if(info->underlying)
    {
        auto u = createInfo(info->underlying);
        mxSetFieldByNumber(r, 0, Field::Underlying, u);
    }

    //
    // Don't use info->type() to determine the type of the EndpointInfo object. When an endpoint is the
    // underlying endpoint of a parent, the child's value for type() is the same as its parent. We have
    // to use type casts instead.
    //

    auto ipInfo = dynamic_pointer_cast<Ice::IPEndpointInfo>(info);
    if(ipInfo)
    {
        mxSetFieldByNumber(r, 0, Field::Host, createStringFromUTF8(ipInfo->host));
        mxSetFieldByNumber(r, 0, Field::Port, createInt(ipInfo->port));
        mxSetFieldByNumber(r, 0, Field::SourceAddress, createStringFromUTF8(ipInfo->sourceAddress));
    }

    auto opaqueInfo = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(info);
    if(opaqueInfo)
    {
        mxSetFieldByNumber(r, 0, Field::RawEncoding, createEncodingVersion(opaqueInfo->rawEncoding));
        Ice::Byte* p = &opaqueInfo->rawBytes[0];
        mxSetFieldByNumber(r, 0, Field::RawBytes, createByteArray(p, p + opaqueInfo->rawBytes.size()));
    }

    auto udpInfo = dynamic_pointer_cast<Ice::UDPEndpointInfo>(info);
    if(udpInfo)
    {
        mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::UDPEndpointType));
        mxSetFieldByNumber(r, 0, Field::McastInterface, createStringFromUTF8(udpInfo->mcastInterface));
        mxSetFieldByNumber(r, 0, Field::McastTtl, createInt(udpInfo->mcastTtl));
    }

    if(dynamic_pointer_cast<Ice::TCPEndpointInfo>(info))
    {
        mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::TCPEndpointType));
    }

    auto wsInfo = dynamic_pointer_cast<Ice::WSEndpointInfo>(info);
    if(wsInfo)
    {
        mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::WSEndpointType));
        mxSetFieldByNumber(r, 0, Field::Resource, createStringFromUTF8(wsInfo->resource));
    }

    if(dynamic_pointer_cast<IceSSL::EndpointInfo>(info))
    {
        mxSetFieldByNumber(r, 0, Field::InfoType, createInt(Ice::SSLEndpointType));
    }

    return r;
}

}

extern "C"
{

mxArray*
Ice_Endpoint_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<Ice::Endpoint>*>(self);
    return 0;
}

mxArray*
Ice_Endpoint_equals(void* self, void* other)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        return createResultValue(createBool(Ice::targetEqualTo(deref<Ice::Endpoint>(self),
                                                               deref<Ice::Endpoint>(other))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Endpoint_toString(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::Endpoint>(self)->toString()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Endpoint_getInfo(void* self)
{
    try
    {
        shared_ptr<Ice::EndpointInfo> info = deref<Ice::Endpoint>(self)->getInfo();
        return createResultValue(createInfo(info));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

}
