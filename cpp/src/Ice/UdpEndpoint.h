// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UDP_ENDPOINT_H
#define ICE_UDP_ENDPOINT_H

#include <Ice/Endpoint.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

const ::Ice::Short UdpEndpointType = 3;

class UdpEndpoint : public Endpoint
{
public:

    UdpEndpoint(const InstancePtr&, const std::string&, ::Ice::Int);
    UdpEndpoint(const InstancePtr&, const std::string&);
    UdpEndpoint(BasicStream*);

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual ::Ice::Short type() const;
    virtual ::Ice::Int timeout() const;
    virtual EndpointPtr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual bool unknown() const;
    virtual TransceiverPtr clientTransceiver() const;
    virtual TransceiverPtr serverTransceiver(EndpointPtr&) const;
    virtual ConnectorPtr connector() const;
    virtual AcceptorPtr acceptor(EndpointPtr&) const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator!=(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const std::string _host;
    const ::Ice::Int _port;
    const bool _connect;
};

class UdpEndpointFactory : public EndpointFactory
{
public:

    virtual ~UdpEndpointFactory();

    virtual ::Ice::Short type() const;
    virtual ::std::string protocol() const;
    virtual EndpointPtr create(const std::string&) const;
    virtual EndpointPtr read(BasicStream*) const;
    virtual void destroy();

private:

    UdpEndpointFactory(const InstancePtr&);
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
