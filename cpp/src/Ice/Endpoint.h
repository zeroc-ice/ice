// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include <IceUtil/Shared.h>
#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>

namespace IceInternal
{

class BasicStream;

const ::Ice::Short UnknownEndpointType = 0;
const ::Ice::Short TcpEndpointType = 1;
const ::Ice::Short SslEndpointType = 2;
const ::Ice::Short UdpEndpointType = 3;
const ::Ice::Short SUdpEndpointType = 4;

class Endpoint : public ::IceUtil::Shared
{
public:

    //
    // Create an endpoint from a string
    //
    static EndpointPtr endpointFromString(const InstancePtr&, const std::string&);
    
    //
    // Unmarshal an endpoint
    //
    static void streamRead(BasicStream*, EndpointPtr&);
    
    //
    // Marshal the endpoint
    //
    virtual void streamWrite(BasicStream*) const = 0;

    //
    // Convert the endpoint to its string form
    //
    virtual std::string toString() const = 0;

    //
    // Return the endpoint type
    //
    virtual ::Ice::Short type() const = 0;
    
    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    virtual ::Ice::Int timeout() const = 0;

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    virtual EndpointPtr timeout(::Ice::Int) const = 0;

    //
    // Return true if the endpoint is datagram-based.
    //
    virtual bool datagram() const = 0;

    //
    // Return true if the endpoint is secure.
    //
    virtual bool secure() const = 0;

    //
    // Return true if the endpoint type is unknown.
    //
    virtual bool unknown() const = 0;

    //
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    virtual TransceiverPtr clientTransceiver() const = 0;

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    virtual TransceiverPtr serverTransceiver(EndpointPtr&) const = 0;

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    virtual ConnectorPtr connector() const = 0;

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    virtual AcceptorPtr acceptor(EndpointPtr&) const = 0;

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    virtual bool equivalent(const TransceiverPtr&) const = 0;
    virtual bool equivalent(const AcceptorPtr&) const = 0;

    //
    // Compare endpoints for sorting purposes
    //
    virtual bool operator==(const Endpoint&) const = 0;
    virtual bool operator!=(const Endpoint&) const = 0;
    virtual bool operator<(const Endpoint&) const = 0;
};

class UnknownEndpoint : public Endpoint
{
public:

    UnknownEndpoint(::Ice::Short, BasicStream*);

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
    ::Ice::Short _type;
    const std::vector< ::Ice::Byte> _rawBytes;
};

class TcpEndpoint : public Endpoint
{
public:

    TcpEndpoint(const InstancePtr&, const std::string&, ::Ice::Int, ::Ice::Int);
    TcpEndpoint(const InstancePtr&, const std::string&);
    TcpEndpoint(BasicStream*);

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
    const ::Ice::Int _timeout;
};

class SslEndpoint : public Endpoint
{
public:

    SslEndpoint(const InstancePtr&, const std::string&, ::Ice::Int, ::Ice::Int);
    SslEndpoint(const InstancePtr&, const std::string&);
    SslEndpoint(BasicStream*);

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
    const ::Ice::Int _timeout;
};

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

class SUdpEndpoint : public Endpoint
{
public:

    SUdpEndpoint(const InstancePtr&, const std::string&, ::Ice::Int);
    SUdpEndpoint(const InstancePtr&, const std::string&);
    SUdpEndpoint(BasicStream*);

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

}

#endif
