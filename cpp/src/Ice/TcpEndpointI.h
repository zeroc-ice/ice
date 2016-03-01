// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/WSEndpoint.h>

namespace IceInternal
{

class TcpEndpointI : public IPEndpointI, public WSEndpointDelegate
{
public:

    TcpEndpointI(const ProtocolInstancePtr&, const std::string&, Ice::Int, const Address&, Ice::Int, const std::string&,
                 bool);
    TcpEndpointI(const ProtocolInstancePtr&);
    TcpEndpointI(const ProtocolInstancePtr&, BasicStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::EndpointInfoPtr getWSInfo(const std::string&) const;

    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;

    virtual TransceiverPtr transceiver() const;
    virtual AcceptorPtr acceptor(const std::string&) const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

    TcpEndpointIPtr endpoint(const TcpAcceptorPtr&) const;

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

    TcpEndpointFactory(const ProtocolInstancePtr&);
    virtual ~TcpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    ProtocolInstancePtr _instance;
};

}

#endif
