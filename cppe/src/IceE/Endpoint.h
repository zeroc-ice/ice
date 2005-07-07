// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSPORT_ENDPOINT_H
#define ICEE_TRANSPORT_ENDPOINT_H

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

const Ice::Short TcpEndpointType = 1;

class ICEE_API Endpoint : public Ice::Shared
{
public:

    Endpoint(const InstancePtr&, const std::string&, Ice::Int, Ice::Int);
    Endpoint(const InstancePtr&, const std::string&);
    Endpoint(BasicStream*);

    void streamWrite(BasicStream*) const;
    std::string toString() const;
    Ice::Short type() const;
    Ice::Int timeout() const;
    EndpointPtr timeout(Ice::Int) const;
    bool unknown() const;
    ConnectorPtr connector() const;
#ifndef ICEE_PURE_CLIENT
    AcceptorPtr acceptor(EndpointPtr&) const;
#endif

    bool equivalent(const TransceiverPtr&) const;
#ifndef ICEE_PURE_CLIENT
    bool equivalent(const AcceptorPtr&) const;
#endif

    bool operator==(const Endpoint&) const;
    bool operator!=(const Endpoint&) const;
    bool operator<(const Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const Ice::Int _timeout;
};

}

#endif
