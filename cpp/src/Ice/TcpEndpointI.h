// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_ENDPOINT_I_H
#define ICE_TCP_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <Ice/IPEndpointI.h>
#include <Ice/EndpointFactory.h>
#include <Ice/Network.h> // for IceIternal::Address

namespace IceInternal
{

class TcpEndpointI : public IPEndpointI
{
public:

    TcpEndpointI(const ProtocolInstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&, bool);
    TcpEndpointI(const ProtocolInstancePtr&);
    TcpEndpointI(const ProtocolInstancePtr&, BasicStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;

    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual TransceiverPtr transceiver(EndpointIPtr&) const;
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

    using IPEndpointI::connectionId;

protected:

    virtual void streamWriteImpl(BasicStream*) const;
    virtual void hashInit(Ice::Int&) const;
    virtual void fillEndpointInfo(Ice::IPEndpointInfo*) const;
    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

    virtual ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const;
    virtual IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const Ice::Int _timeout;
    const bool _compress;
};

class TcpEndpointFactory : public EndpointFactory
{
public:

    virtual ~TcpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    TcpEndpointFactory(const ProtocolInstancePtr&);
    friend class Instance;

    ProtocolInstancePtr _instance;
};
    
}

#endif
