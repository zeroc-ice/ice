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
#include <Ice/winrt/StreamF.h>

namespace IceInternal
{

class StreamEndpointI : public IPEndpointI
{
public:

    StreamEndpointI(const ProtocolInstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&, bool);
    StreamEndpointI(const ProtocolInstancePtr&);
    StreamEndpointI(const ProtocolInstancePtr&, BasicStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;

    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual TransceiverPtr transceiver() const;
    virtual AcceptorPtr acceptor(const std::string&) const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

    StreamEndpointIPtr endpoint(const StreamAcceptorPtr&) const;

    using IPEndpointI::connectionId;

protected:

    virtual void streamWriteImpl(BasicStream*) const;
    virtual void hashInit(Ice::Int&) const;
    virtual void initWithOptions(std::vector<std::string>&, bool);
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

class StreamEndpointFactory : public EndpointFactory
{
public:

    virtual ~StreamEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    StreamEndpointFactory(const ProtocolInstancePtr&);
    friend class Instance;

    ProtocolInstancePtr _instance;
};

}

#endif
