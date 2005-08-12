// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UNKNOWN_ENDPOINT_H
#define ICE_UNKNOWN_ENDPOINT_H

#include <IceE/Endpoint.h>

namespace IceInternal
{

class UnknownEndpoint : public Endpoint
{
public:

    UnknownEndpoint(Ice::Short, BasicStream*);

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::Short type() const;
    virtual Ice::Int timeout() const;
    virtual EndpointPtr timeout(Ice::Int) const;
    virtual bool unknown() const;
    virtual ConnectorPtr connector() const;
#ifndef ICE_PURE_CLIENT
    virtual AcceptorPtr acceptor(EndpointPtr&) const;
#endif
    virtual bool equivalent(const TransceiverPtr&) const;
#ifndef ICE_PURE_CLIENT
    virtual bool equivalent(const AcceptorPtr&) const;
#endif

    virtual bool operator==(const Endpoint&) const;
    virtual bool operator!=(const Endpoint&) const;
    virtual bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    Ice::Short _type;
    const std::vector<Ice::Byte> _rawBytes;
};

}

#endif
