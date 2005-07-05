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

#include <IceE/Shared.h>
#include <IceE/EndpointF.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/AcceptorF.h>
#endif
#include <IceE/ConnectorF.h>
#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>

namespace IceEInternal
{

class BasicStream;

const IceE::Short TcpEndpointType = 1;

//
// For UDP style transports there is no connector or acceptor.
//
//#define ICEE_TRANSPORT_NO_CONNECT

class ICEE_PROTOCOL_API Endpoint : public IceE::Shared
{
public:

    Endpoint(const InstancePtr&, const std::string&, IceE::Int, IceE::Int);
    Endpoint(const InstancePtr&, const std::string&);
    Endpoint(BasicStream*);

    void streamWrite(BasicStream*) const;
    std::string toString() const;
    IceE::Short type() const;
    IceE::Int timeout() const;
    EndpointPtr timeout(IceE::Int) const;
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
    const IceE::Int _port;
    const IceE::Int _timeout;
};

}

#endif
