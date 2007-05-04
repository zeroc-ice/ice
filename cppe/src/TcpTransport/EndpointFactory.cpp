// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/EndpointFactory.h>
#include <IceE/UnknownEndpoint.h>
#include <IceE/LocalException.h>
#include <IceE/BasicStream.h>
#include <TcpTransport/TcpEndpoint.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(EndpointFactory* p) { return p; }

IceInternal::EndpointFactory::EndpointFactory(const InstancePtr& instance)
    : _instance(instance)
{
}

IceInternal::EndpointFactory::~EndpointFactory()
{
}

EndpointPtr
IceInternal::EndpointFactory::create(const std::string& str) const
{
    const string delim = " \t\n\r";

    string::size_type beg = str.find_first_not_of(delim);
    if(beg == string::npos)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = str;
        throw ex;
    }

    string::size_type end = str.find_first_of(delim, beg);
    if(end == string::npos)
    {
        end = str.length();
    }

    string protocol = str.substr(beg, end - beg);

    if(protocol == "default" || protocol == "tcp")
    {
	return new TcpEndpoint(_instance, str.substr(end));
    }

    return 0;
}

EndpointPtr
IceInternal::EndpointFactory::read(BasicStream* s) const
{
    Short type;
    s->read(type);

    if(type == TcpEndpointType)
    {
        return new TcpEndpoint(s);
    }

    return new UnknownEndpoint(type, s);
}

void
IceInternal::EndpointFactory::destroy()
{
    _instance = 0;
}
