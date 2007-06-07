// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_ENDPOINT_I_H
#define ICE_UDP_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

const Ice::Short UdpEndpointType = 3;

class UdpEndpointI : public EndpointI
{
public:

    UdpEndpointI(const InstancePtr&, const std::string&, Ice::Int, const std::string&, Ice::Int, bool,
                 const std::string&, bool, bool);
    UdpEndpointI(const InstancePtr&, const std::string&, bool);
    UdpEndpointI(BasicStream*);

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::Short type() const;
    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual bool unknown() const;
    virtual TransceiverPtr transceiver(EndpointIPtr&) const;
    virtual std::vector<ConnectorPtr> connectors() const;
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const;
    virtual std::vector<EndpointIPtr> expand() const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const EndpointI&) const;
    virtual bool operator!=(const EndpointI&) const;
    virtual bool operator<(const EndpointI&) const;

private:

#if defined(__SUNPRO_CC)
    //
    // COMPILERFIX: prevent the compiler from emitting a warning about
    // hidding these operators.
    //
    using LocalObject::operator==;
    using LocalObject::operator!=;
    using LocalObject::operator<;
#endif

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const std::string _mcastInterface;
    const Ice::Int _mcastTtl;
    const Ice::Byte _protocolMajor;
    const Ice::Byte _protocolMinor;
    const Ice::Byte _encodingMajor;
    const Ice::Byte _encodingMinor;
    const bool _connect;
    const std::string _connectionId;
    const bool _compress;
    const bool _oaEndpoint;
};

class UdpEndpointFactory : public EndpointFactory
{
public:

    virtual ~UdpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(const std::string&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

private:

    UdpEndpointFactory(const InstancePtr&);
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
