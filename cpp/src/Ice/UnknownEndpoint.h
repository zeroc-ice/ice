// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UNKNOWN_ENDPOINT_H
#define ICE_UNKNOWN_ENDPOINT_H

#include <Ice/Endpoint.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

const ::Ice::Short UnknownEndpointType = 0;

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

}

#endif
