// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UNKNOWN_ENDPOINT_I_H
#define ICE_UNKNOWN_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

class OpaqueEndpointI : public EndpointI, public Ice::OpaqueEndpoint
{
public:

    OpaqueEndpointI(const ::std::string&);
    OpaqueEndpointI(Ice::Short, BasicStream*);

    // From OpaqueEndpoint
    virtual std::string toString() const;
    virtual Ice::Int timeout() const;
    virtual bool compress() const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Ice::ByteSeq rawBytes() const;

    // From EndpointI
    virtual void streamWrite(BasicStream*) const;
    virtual Ice::Short type() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual EndpointIPtr compress(bool) const;
    virtual TransceiverPtr transceiver(EndpointIPtr&) const;
    virtual std::vector<ConnectorPtr> connectors() const;
    virtual void connectors_async(const EndpointI_connectorsPtr&) const;
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const;
    virtual std::vector<EndpointIPtr> expand() const;
    virtual bool equivalent(const EndpointIPtr&) const;

    virtual bool operator==(const EndpointI&) const;
    virtual bool operator!=(const EndpointI&) const;
    virtual bool operator<(const EndpointI&) const;

#if defined(__BCPLUSPLUS__)
    //
    // COMPILERFIX: Avoid warnings about hiding members for C++Builder 2010
    //
    //
    virtual bool operator==(const Ice::LocalObject& rhs) const
    {
        return EndpointI::operator==(rhs);
    }

    virtual bool operator<(const Ice::LocalObject& rhs) const
    {
        return EndpointI::operator<(rhs);
    }

    virtual std::vector<ConnectorPtr> connectors(const std::vector<struct sockaddr_storage>& v) const
    {
        return EndpointI::connectors(v);
    }
#endif

protected:
    using EndpointI::connectors;

private:

#if defined(__SUNPRO_CC)
    //
    // COMPILERFIX: prevent the compiler from emitting a warning about
    // hidding these operators.
    //
    using LocalObject::operator==;
    using LocalObject::operator<;
#endif

    //
    // All members are const, because endpoints are immutable.
    //
    Ice::Short _type;
    const std::vector<Ice::Byte> _rawBytes;
};

}

#endif
