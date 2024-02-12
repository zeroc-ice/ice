//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TCP_ENDPOINT_I_H
#define ICE_TCP_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <Ice/IPEndpointI.h>
#include <Ice/EndpointFactory.h>
#include <Ice/Network.h> // for IceIternal::Address

namespace IceInternal
{

class TcpEndpointI final : public IPEndpointI
{
public:

    TcpEndpointI(const ProtocolInstancePtr&, const std::string&, Ice::Int, const Address&, Ice::Int, const std::string&,
                 bool);
    TcpEndpointI(const ProtocolInstancePtr&);
    TcpEndpointI(const ProtocolInstancePtr&, Ice::InputStream*);

    void streamWriteImpl(Ice::OutputStream*) const final;

    Ice::EndpointInfoPtr getInfo() const noexcept final;

    Ice::Int timeout() const final;
    EndpointIPtr timeout(Ice::Int) const final;
    bool compress() const final;
    EndpointIPtr compress(bool) const final;
    bool datagram() const final;

    TransceiverPtr transceiver() const final;
    AcceptorPtr acceptor(const std::string&) const final;
    std::string options() const final;

    bool operator==(const Ice::Endpoint&) const final;
    bool operator<(const Ice::Endpoint&) const final;
    TcpEndpointIPtr endpoint(const TcpAcceptorPtr&) const;

    using IPEndpointI::connectionId;

protected:

    void hashInit(Ice::Int&) const final;
    void fillEndpointInfo(Ice::IPEndpointInfo*) const final;
    bool checkOption(const std::string&, const std::string&, const std::string&) final;

    ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const final;
    IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const final;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const Ice::Int _timeout;
    const bool _compress;
};

class TcpEndpointFactory final : public EndpointFactory
{
public:

    TcpEndpointFactory(const ProtocolInstancePtr&);
    ~TcpEndpointFactory();

    Ice::Short type() const final;
    std::string protocol() const final;
    EndpointIPtr create(std::vector<std::string>&, bool) const final;
    EndpointIPtr read(Ice::InputStream*) const final;
    void destroy() final;

    EndpointFactoryPtr clone(const ProtocolInstancePtr&) const final;

private:

    ProtocolInstancePtr _instance;
};

}

#endif
