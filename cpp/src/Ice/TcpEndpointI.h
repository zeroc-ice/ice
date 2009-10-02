// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_ENDPOINT_I_H
#define ICE_TCP_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

const Ice::Short TcpEndpointType = 1;

class TcpEndpointI : public EndpointI, public Ice::TcpEndpoint
{
public:

    TcpEndpointI(const InstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&, bool);
    TcpEndpointI(const InstancePtr&, const std::string&, bool);
    TcpEndpointI(BasicStream*);

    // From TcpEndpoint
    virtual std::string toString() const;
    virtual Ice::Int timeout() const;
    virtual bool compress() const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual std::string host() const;
    virtual Ice::Int port() const;

    // From EndpointI
    virtual void streamWrite(BasicStream*) const;
    virtual Ice::Short type() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual EndpointIPtr compress(bool) const;
    virtual TransceiverPtr transceiver(EndpointIPtr&) const;
    virtual std::vector<ConnectorPtr> connectors() const;
    virtual void connectors_async(const EndpointI_connectorsPtr&) const;
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const;
    virtual std::vector<EndpointIPtr> expand() const;
    virtual bool equivalent(const EndpointIPtr&) const;

    virtual bool operator==(const EndpointI&) const;
    virtual bool operator!=(const EndpointI&) const;
    virtual bool operator<(const EndpointI&) const;

#if defined(__BCPLUSPLUS__)
    //
    // COMPILERFIX: Avoid warnings about hiding members for C++Builder 2010
    //
    //
    virtual bool operator==(const Ice::LocalObject& rhs) const
    {
        return EndpointI::operator==(rhs);
    }

    virtual bool operator<(const Ice::LocalObject& rhs) const
    {
        return EndpointI::operator<(rhs);
    }
#endif

private:

    virtual std::vector<ConnectorPtr> connectors(const std::vector<struct sockaddr_storage>&) const;

#if defined(__SUNPRO_CC)
    //
    // COMPILERFIX: prevent the compiler from emitting a warning about
    // hidding these operators.
    //
    using LocalObject::operator==;
    using LocalObject::operator<;
#endif

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const Ice::Int _timeout;
    const std::string _connectionId;
    const bool _compress;
};

class TcpEndpointFactory : public EndpointFactory
{
public:

    virtual ~TcpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(const std::string&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

private:

    TcpEndpointFactory(const InstancePtr&);
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
