// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_H
#define ICE_ENDPOINT_I_H

#include <IceUtil/Shared.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointIF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>

namespace IceInternal
{

class BasicStream;

class ICE_API EndpointI : public Ice::Endpoint
{
public:

    //
    // Marshal the endpoint.
    //
    virtual void streamWrite(BasicStream*) const = 0;

    //
    // Return the endpoint type.
    //
    virtual Ice::Short type() const = 0;
    
    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    virtual Ice::Int timeout() const = 0;

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    virtual EndpointIPtr timeout(Ice::Int) const = 0;

    //
    // Return a new endpoint with a different connection id.
    //
    virtual EndpointIPtr connectionId(const ::std::string&) const = 0;

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    virtual bool compress() const = 0;

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    virtual EndpointIPtr compress(bool) const = 0;

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
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    virtual TransceiverPtr transceiver(EndpointIPtr&) const = 0;

    //
    // Return connectors for this endpoint, or empty vector if no 
    // connector is available.
    //
    virtual std::vector<ConnectorPtr> connectors() const = 0;

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const = 0;

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    virtual std::vector<EndpointIPtr> expand() const = 0;

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor.
    //
    virtual bool equivalent(const TransceiverPtr&) const = 0;
    virtual bool equivalent(const AcceptorPtr&) const = 0;

    //
    // Compare endpoints for sorting purposes.
    //
    virtual bool operator==(const EndpointI&) const = 0;
    virtual bool operator!=(const EndpointI&) const = 0;
    virtual bool operator<(const EndpointI&) const = 0;

private:

#if defined(__SUNPRO_CC) || defined(__HP_aCC)
    //
    // COMPILERFIX: prevent the compiler from emitting a warning about
    // hidding these operators.
    //
    using LocalObject::operator==;
    using LocalObject::operator!=;
    using LocalObject::operator<;
#endif
};

}

#endif
