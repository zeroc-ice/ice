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

class ICE_API EndpointI : public Shared
{
public:

    EndpointI() { }
    
    //
    // Return true if the endpoint only supports oneway operations.
    //
    virtual bool oneway() const = 0;

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    virtual int timeout() const = 0;

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    virtual Endpoint timeout(int) const = 0;

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
    // or null if a Transceiver can only be created by Acceptors or
    // Connectors.
    //
    virtual Transceiver clientTransceiver(const Instance&) const = 0;
    virtual Transceiver serverTransceiver(const Instance&) const = 0;

    //
    // Return Acceptors and Connectors for the endpoint, or null if no
    // Acceptors and Connectors are available.
    //
    virtual Connector connector(const Instance&) const = 0;
    virtual Acceptor acceptor(const Instance&) const = 0;

    //
    // Compare endpoints for sorting purposes
    //
    virtual bool operator==(const EndpointI&) const = 0;
    virtual bool operator!=(const EndpointI&) const = 0;
    virtual bool operator<(const EndpointI&) const = 0;

private:

    EndpointI(const EndpointI&);
    void operator=(const EndpointI&);
};

class ICE_API TcpEndpointI : public EndpointI
{
public:

    TcpEndpointI(const std::string&, int, int);

    virtual bool oneway() const;
    virtual int timeout() const;
    virtual Endpoint timeout(int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Transceiver clientTransceiver(const Instance&) const;
    virtual Transceiver serverTransceiver(const Instance&) const;
    virtual Connector connector(const Instance&) const;
    virtual Acceptor acceptor(const Instance&) const;

    virtual bool operator==(const EndpointI&) const;
    virtual bool operator!=(const EndpointI&) const;
    virtual bool operator<(const EndpointI&) const;

private:

    TcpEndpointI(const TcpEndpointI&);
    void operator=(const TcpEndpointI&);

    //
    // All  members are const, because TcpEndpoints are immutable.
    //
    const std::string host_;
    const int port_;
    const int timeout_;
};

class ICE_API UdpEndpointI : public EndpointI
{
public:

    UdpEndpointI(const std::string&, int);

    virtual bool oneway() const;
    virtual int timeout() const;
    virtual Endpoint timeout(int) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual Transceiver clientTransceiver(const Instance&) const;
    virtual Transceiver serverTransceiver(const Instance&) const;
    virtual Connector connector(const Instance&) const;
    virtual Acceptor acceptor(const Instance&) const;

    virtual bool operator==(const EndpointI&) const;
    virtual bool operator!=(const EndpointI&) const;
    virtual bool operator<(const EndpointI&) const;

private:

    UdpEndpointI(const UdpEndpointI&);
    void operator=(const UdpEndpointI&);

    //
    // All  members are const, because UdpEndpoints are immutable.
    //
    const std::string host_;
    const int port_;
};

}

#endif
