// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UnknownEndpoint.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UnknownEndpoint::UnknownEndpoint(Short type, BasicStream* s) :
    _instance(s->instance()),
    _type(type)
{
    s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::UnknownEndpoint::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::UnknownEndpoint::toString() const
{
    return string();
}

Short
IceInternal::UnknownEndpoint::type() const
{
    return _type;
}

Int
IceInternal::UnknownEndpoint::timeout() const
{
    return -1;
}

EndpointPtr
IceInternal::UnknownEndpoint::timeout(Int) const
{
    return const_cast<UnknownEndpoint*>(this);
}

bool
IceInternal::UnknownEndpoint::compress() const
{
    return false;
}

EndpointPtr
IceInternal::UnknownEndpoint::compress(bool) const
{
    return const_cast<UnknownEndpoint*>(this);
}

bool
IceInternal::UnknownEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::secure() const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::unknown() const
{
    return true;
}

TransceiverPtr
IceInternal::UnknownEndpoint::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceInternal::UnknownEndpoint::serverTransceiver(EndpointPtr& endp) const
{
    endp = const_cast<UnknownEndpoint*>(this);
    return 0;
}

ConnectorPtr
IceInternal::UnknownEndpoint::connector() const
{
    return 0;
}

AcceptorPtr
IceInternal::UnknownEndpoint::acceptor(EndpointPtr& endp) const
{
    endp = const_cast<UnknownEndpoint*>(this);
    return 0;
}

bool
IceInternal::UnknownEndpoint::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::operator==(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if(!p)
    {
	return false;
    }

    if(this == p)
    {
	return true;
    }

    if(_type != p->_type)
    {
        return false;
    }

    if(_rawBytes != p->_rawBytes)
    {
	return false;
    }

    return true;
}

bool
IceInternal::UnknownEndpoint::operator!=(const Endpoint& r) const
{
    return !operator==(r);
}

bool
IceInternal::UnknownEndpoint::operator<(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
	return false;
    }

    if(_type < p->_type)
    {
	return true;
    }
    else if(p->_type < _type)
    {
	return false;
    }

    if(_rawBytes < p->_rawBytes)
    {
	return true;
    }
    else if(p->_rawBytes < _rawBytes)
    {
	return false;
    }

    return false;
}
