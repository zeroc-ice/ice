// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UnknownEndpointI.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::UnknownEndpointI::UnknownEndpointI(Short type, BasicStream* s) :
    _instance(s->instance()),
    _type(type)
{
    s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::UnknownEndpointI::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::UnknownEndpointI::toString() const
{
    return string();
}

Short
IceInternal::UnknownEndpointI::type() const
{
    return _type;
}

Int
IceInternal::UnknownEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::UnknownEndpointI::timeout(Int) const
{
    return const_cast<UnknownEndpointI*>(this);
}

EndpointIPtr
IceInternal::UnknownEndpointI::connectionId(const string&) const
{
    return const_cast<UnknownEndpointI*>(this);
}

bool
IceInternal::UnknownEndpointI::compress() const
{
    return false;
}

EndpointIPtr
IceInternal::UnknownEndpointI::compress(bool) const
{
    return const_cast<UnknownEndpointI*>(this);
}

bool
IceInternal::UnknownEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::secure() const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::unknown() const
{
    return true;
}

TransceiverPtr
IceInternal::UnknownEndpointI::clientTransceiver() const
{
    return 0;
}

TransceiverPtr
IceInternal::UnknownEndpointI::serverTransceiver(EndpointIPtr& endp) const
{
    endp = const_cast<UnknownEndpointI*>(this);
    return 0;
}

ConnectorPtr
IceInternal::UnknownEndpointI::connector() const
{
    return 0;
}

AcceptorPtr
IceInternal::UnknownEndpointI::acceptor(EndpointIPtr& endp) const
{
    endp = const_cast<UnknownEndpointI*>(this);
    return 0;
}

vector<EndpointIPtr>
IceInternal::UnknownEndpointI::expand() const
{
    assert(false);
    vector<EndpointIPtr> ret;
    return ret;
}

bool
IceInternal::UnknownEndpointI::publish() const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::equivalent(const TransceiverPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::equivalent(const AcceptorPtr&) const
{
    return false;
}

bool
IceInternal::UnknownEndpointI::operator==(const EndpointI& r) const
{
    const UnknownEndpointI* p = dynamic_cast<const UnknownEndpointI*>(&r);
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
IceInternal::UnknownEndpointI::operator!=(const EndpointI& r) const
{
    return !operator==(r);
}

bool
IceInternal::UnknownEndpointI::operator<(const EndpointI& r) const
{
    const UnknownEndpointI* p = dynamic_cast<const UnknownEndpointI*>(&r);
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
