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

#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Stream;

const ::Ice::Short UnknownEndpointType = 0;
const ::Ice::Short TcpEndpointType = 1;
const ::Ice::Short UdpEndpointType = 2;

class Endpoint : public Shared
{
public:

    Endpoint() { }

    //
    // Create an endpoint from a string
    //
    static Endpoint_ptr endpointFromString(const std::string&);
    
    //
    // Unmarshal an endpoint
    //
    static void streamRead(Stream*, Endpoint_ptr&);
    
    //
    // Marshal the endpoint
    //
    virtual void streamWrite(Stream*) const = 0;

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
    virtual Endpoint_ptr timeout(::Ice::Int) const = 0;

    //
    // Return true if the endpoint is datagram-based.
    //
    virtual bool datagram() const = 0;

    //
    // Return true if the endpoint is secure.
    //
    virtual bool secure() const = 0;

    //
    // Return client- and server-side Transceivers for the endpoint,
    // or null if a Transceiver_ptr can only be created by Acceptors or
    // Connectors.
    //
    virtual Transceiver_ptr clientTransceiver(const Instance_ptr&) const = 0;
    virtual Transceiver_ptr serverTransceiver(const Instance_ptr&) const = 0;

    //
    // Return Acceptors and Connectors for the endpoint, or null if no
    // Acceptors and Connectors are available.
    //
    virtual Connector_ptr connector(const Instance_ptr&) const = 0;
    virtual Acceptor_ptr acceptor(const Instance_ptr&) const = 0;

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

    UnknownEndpoint(Stream*);

    virtual void streamWrite(Stream*) const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual Endpoint_ptr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Transceiver_ptr clientTransceiver(const Instance_ptr&) const;
    virtual Transceiver_ptr serverTransceiver(const Instance_ptr&) const;
    virtual Connector_ptr connector(const Instance_ptr&) const;
    virtual Acceptor_ptr acceptor(const Instance_ptr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator!=(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All  members are const, because TcpEndpoints are immutable.
    //
    const std::vector< ::Ice::Byte> rawBytes_;
};

class TcpEndpoint : public Endpoint
{
public:

    TcpEndpoint(const std::string&, ::Ice::Int, ::Ice::Int);
    TcpEndpoint(const std::string&);
    TcpEndpoint(Stream*);

    virtual void streamWrite(Stream*) const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual Endpoint_ptr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Transceiver_ptr clientTransceiver(const Instance_ptr&) const;
    virtual Transceiver_ptr serverTransceiver(const Instance_ptr&) const;
    virtual Connector_ptr connector(const Instance_ptr&) const;
    virtual Acceptor_ptr acceptor(const Instance_ptr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator!=(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All  members are const, because TcpEndpoints are immutable.
    //
    const std::string host_;
    const ::Ice::Int port_;
    const ::Ice::Int timeout_;
};

class UdpEndpoint : public Endpoint
{
public:

    UdpEndpoint(const std::string&, ::Ice::Int);
    UdpEndpoint(const std::string&);
    UdpEndpoint(Stream*);

    virtual void streamWrite(Stream*) const;
    virtual ::Ice::Short type() const;
    virtual bool oneway() const;
    virtual ::Ice::Int timeout() const;
    virtual Endpoint_ptr timeout(::Ice::Int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Transceiver_ptr clientTransceiver(const Instance_ptr&) const;
    virtual Transceiver_ptr serverTransceiver(const Instance_ptr&) const;
    virtual Connector_ptr connector(const Instance_ptr&) const;
    virtual Acceptor_ptr acceptor(const Instance_ptr&) const;

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator!=(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All  members are const, because UdpEndpoints are immutable.
    //
    const std::string host_;
    const ::Ice::Int port_;
};

}

#endif
