// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ENDPOINT_H
#define ICEE_ENDPOINT_H

#include <IceE/EndpointF.h>
#include <IceE/ConnectorF.h>
#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/LoggerF.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/AcceptorF.h>
#endif

#include <IceE/Shared.h>

namespace IceInternal
{

class BasicStream;

class ICEE_API Endpoint : public IceUtil::Shared
{
public:

    //
    // Marshal the endpoint.
    //
    virtual void streamWrite(BasicStream*) const = 0;

    //
    // Convert the endpoint to its string form.
    //
    virtual std::string toString() const = 0;

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
    virtual EndpointPtr timeout(Ice::Int) const = 0;
    
    //
    // Return true if the endpoint type is unknown.
    //
    virtual bool unknown() const = 0;

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
#ifndef ICEE_PURE_CLIENT
    virtual AcceptorPtr acceptor(EndpointPtr&) const = 0;
#endif

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor.
    //
    virtual bool equivalent(const TransceiverPtr&) const = 0;
#ifndef ICEE_PURE_CLIENT
    virtual bool equivalent(const AcceptorPtr&) const = 0;
#endif

    //
    // Compare endpoints for sorting purposes.
    //
    virtual bool operator==(const Endpoint&) const = 0;
    virtual bool operator!=(const Endpoint&) const = 0;
    virtual bool operator<(const Endpoint&) const = 0;
};

}

#endif
