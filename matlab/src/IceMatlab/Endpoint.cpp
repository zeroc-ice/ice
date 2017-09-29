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
#include "Util.h"

#define DEREF(x) (*(reinterpret_cast<shared_ptr<Ice::Endpoint>*>(x)))
#define SELF DEREF(self)

using namespace std;
using namespace IceMatlab;

void*
IceMatlab::createEndpoint(shared_ptr<Ice::Endpoint> p)
{
    return new shared_ptr<Ice::Endpoint>(p);
}

shared_ptr<Ice::Endpoint>
IceMatlab::getEndpoint(void* p)
{
    return DEREF(p);
}

namespace
{

enum Field
{
    Type = 0,
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
createInfo(shared_ptr<Ice::EndpointInfo> info)
{
    //
    // Create and return a struct array containing the fields that describe the EndpointInfo object.
    // Some fields will be left unused.
    //

    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, Field::NumFields, infoFields);
    mxSetFieldByNumber(r, 0, Field::Type, createInt(info->type())); // May be overridden below.
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

    shared_ptr<Ice::IPEndpointInfo> ipInfo = dynamic_pointer_cast<Ice::IPEndpointInfo>(info);
    if(ipInfo)
    {
        mxSetFieldByNumber(r, 0, Field::Host, createStringFromUTF8(ipInfo->host));
        mxSetFieldByNumber(r, 0, Field::Port, createInt(ipInfo->port));
        mxSetFieldByNumber(r, 0, Field::SourceAddress, createStringFromUTF8(ipInfo->sourceAddress));
    }

    shared_ptr<Ice::OpaqueEndpointInfo> opaqueInfo = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(info);
    if(opaqueInfo)
    {
        mxSetFieldByNumber(r, 0, Field::RawEncoding, createEncodingVersion(opaqueInfo->rawEncoding));
        Ice::Byte* p = &opaqueInfo->rawBytes[0];
        mxSetFieldByNumber(r, 0, Field::RawBytes, createByteArray(p, p + opaqueInfo->rawBytes.size()));
    }

    shared_ptr<Ice::UDPEndpointInfo> udpInfo = dynamic_pointer_cast<Ice::UDPEndpointInfo>(info);
    if(udpInfo)
    {
        mxSetFieldByNumber(r, 0, Field::Type, createInt(Ice::UDPEndpointType));
        mxSetFieldByNumber(r, 0, Field::McastInterface, createStringFromUTF8(udpInfo->mcastInterface));
        mxSetFieldByNumber(r, 0, Field::McastTtl, createInt(udpInfo->mcastTtl));
    }

    if(dynamic_pointer_cast<Ice::TCPEndpointInfo>(info))
    {
        mxSetFieldByNumber(r, 0, Field::Type, createInt(Ice::TCPEndpointType));
    }

    shared_ptr<Ice::WSEndpointInfo> wsInfo = dynamic_pointer_cast<Ice::WSEndpointInfo>(info);
    if(wsInfo)
    {
        mxSetFieldByNumber(r, 0, Field::Resource, createStringFromUTF8(wsInfo->resource));
    }

    /* TODO: If we link with IceSSL
    if(dynamic_pointer_cast<IceSSL::EndpointInfo>(info))
    {
        mxSetFieldByNumber(r, 0, Field::Type, createInt(Ice::SSLEndpointType));
    }
    */

    return r;
}

}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_Endpoint__release(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Endpoint_equals(void* self, void* other)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        return createResultValue(createBool(Ice::targetEqualTo(SELF, DEREF(other))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

EXPORTED_FUNCTION mxArray*
Ice_Endpoint_toString(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->toString()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

EXPORTED_FUNCTION mxArray*
Ice_Endpoint_getInfo(void* self)
{
    try
    {
        shared_ptr<Ice::EndpointInfo> info = SELF->getInfo();
        return createResultValue(createInfo(info));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

}
