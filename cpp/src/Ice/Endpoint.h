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

class IntStream;

const ::Ice::Short UnknownEndpointType = 0;
const ::Ice::Short TcpEndpointType = 1;
const ::Ice::Short SslEndpointType = 2;
const ::Ice::Short UdpEndpointType = 3;

class Endpoint : public ::IceUtil::Shared
{
public:

    Endpoint() { }

    //
    // Create an endpoint from a string
    //
    static EndpointPtr endpointFromString(const std::string&);
    
    //
    // Unmarshal an endpoint
    //
    static void streamRead(IntStream*, EndpointPtr&);
    
    //
    // Marshal the endpoint
    //
    virtual void streamWrite(IntStream*) const = 0;

    //
    // Convert the endpoint to its string form
    //
    virtual std::string toString() const = 0;

    //
    // Return the endpoint type
    //
    virtual ::Ice::Short type() const = 0;
    
    //
    // Return true if the endpoint only supports oneway operations.
    //
    virtual bool oneway() const = 0;

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
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    virtual TransceiverPtr clientTransceiver(const InstancePtr&) const = 0;

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    virtual TransceiverPtr serverTransceiver(const InstancePtr&, EndpointPtr&) const = 0;

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    virtual ConnectorPtr connector(const InstancePtr&) const = 0;

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    virtual AcceptorPtr acceptor(const InstancePtr&, EndpointPtr&) const = 0;

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
    virtual bool operator<(const Endpoint&) const = 0;
};

class UnknownEndpoint : public Endpoint
{
public:

    UnknownEndpoint(IntStream*);

    virtual void streamWrite(IntStream*) const;
    virtual std::string toString() const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual EndpointPtr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual TransceiverPtr clientTransceiver(const InstancePtr&) const;
    virtual TransceiverPtr serverTransceiver(const InstancePtr&, EndpointPtr&) const;
    virtual ConnectorPtr connector(const InstancePtr&) const;
    virtual AcceptorPtr acceptor(const InstancePtr&, EndpointPtr&) const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const std::vector< ::Ice::Byte> _rawBytes;
};

class TcpEndpoint : public Endpoint
{
public:

    TcpEndpoint(const std::string&, ::Ice::Int, ::Ice::Int);
    TcpEndpoint(const std::string&);
    TcpEndpoint(IntStream*);

    virtual void streamWrite(IntStream*) const;
    virtual std::string toString() const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual EndpointPtr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual TransceiverPtr clientTransceiver(const InstancePtr&) const;
    virtual TransceiverPtr serverTransceiver(const InstancePtr&, EndpointPtr&) const;
    virtual ConnectorPtr connector(const InstancePtr&) const;
    virtual AcceptorPtr acceptor(const InstancePtr&, EndpointPtr&) const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const std::string _host;
    const ::Ice::Int _port;
    const ::Ice::Int _timeout;
};

class SslEndpoint : public Endpoint
{
public:

    SslEndpoint(const std::string&, ::Ice::Int, ::Ice::Int);
    SslEndpoint(const std::string&);
    SslEndpoint(IntStream*);

    virtual void streamWrite(IntStream*) const;
    virtual std::string toString() const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual EndpointPtr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual TransceiverPtr clientTransceiver(const InstancePtr&) const;
    virtual TransceiverPtr serverTransceiver(const InstancePtr&, EndpointPtr&) const;
    virtual ConnectorPtr connector(const InstancePtr&) const;
    virtual AcceptorPtr acceptor(const InstancePtr&, EndpointPtr&) const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const std::string _host;
    const ::Ice::Int _port;
    const ::Ice::Int _timeout;
};

class UdpEndpoint : public Endpoint
{
public:

    UdpEndpoint(const std::string&, ::Ice::Int);
    UdpEndpoint(const std::string&);
    UdpEndpoint(IntStream*);

    virtual void streamWrite(IntStream*) const;
    virtual std::string toString() const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual EndpointPtr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual TransceiverPtr clientTransceiver(const InstancePtr&) const;
    virtual TransceiverPtr serverTransceiver(const InstancePtr&, EndpointPtr&) const;
    virtual ConnectorPtr connector(const InstancePtr&) const;
    virtual AcceptorPtr acceptor(const InstancePtr&, EndpointPtr&) const;
    virtual bool equivalent(const TransceiverPtr&) const;
    virtual bool equivalent(const AcceptorPtr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const std::string _host;
    const ::Ice::Int _port;
};

}

#endif
