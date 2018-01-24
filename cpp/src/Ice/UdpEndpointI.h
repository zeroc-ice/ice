// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_ENDPOINT_I_H
#define ICE_UDP_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <Ice/IPEndpointI.h>
#include <Ice/EndpointFactory.h>
#include <Ice/Network.h>

namespace IceInternal
{

class UdpEndpointI : public IPEndpointI
{
public:

    UdpEndpointI(const ProtocolInstancePtr&, const std::string&, Ice::Int, const Address&, const std::string&,
                 Ice::Int, bool, const std::string&, bool);
    UdpEndpointI(const ProtocolInstancePtr&);
    UdpEndpointI(const ProtocolInstancePtr&, Ice::InputStream*);

    virtual void streamWriteImpl(Ice::OutputStream*) const;

    virtual Ice::EndpointInfoPtr getInfo() const ICE_NOEXCEPT;

    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;

    virtual TransceiverPtr transceiver() const;
    virtual AcceptorPtr acceptor(const std::string&) const;
    virtual std::string options() const;

#ifdef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;
#else
    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;
#endif

    UdpEndpointIPtr endpoint(const UdpTransceiverPtr&) const;

    using IPEndpointI::connectionId;

    virtual void initWithOptions(std::vector<std::string>&, bool);

protected:

    virtual void hashInit(Ice::Int&) const;
    virtual void fillEndpointInfo(Ice::IPEndpointInfo*) const;
    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

    virtual ConnectorPtr createConnector(const Address&, const NetworkProxyPtr&) const;
    virtual IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const Ice::Int _mcastTtl;
    const std::string _mcastInterface;
    const bool _connect;
    const bool _compress;
};

class UdpEndpointFactory : public EndpointFactory
{
public:

    UdpEndpointFactory(const ProtocolInstancePtr&);
    virtual ~UdpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    ProtocolInstancePtr _instance;
};

}

#endif
