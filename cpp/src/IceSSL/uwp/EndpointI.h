// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UWP_ENDPOINT_I_H
#define ICE_SSL_UWP_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <Ice/IPEndpointI.h>
#include <Ice/EndpointFactory.h>
#include <Ice/WSEndpoint.h>
#include <Ice/Network.h> // for IceIternal::Address
#include <IceSSL/uwp/TransceiverF.h>

namespace IceSSL
{

class EndpointI : public IceInternal::IPEndpointI, public IceInternal::WSEndpointDelegate
{
public:

    EndpointI(const IceInternal::ProtocolInstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&, bool);
    EndpointI(const IceInternal::ProtocolInstancePtr&);
    EndpointI(const IceInternal::ProtocolInstancePtr&, Ice::InputStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::EndpointInfoPtr getWSInfo(const std::string&) const;

    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::string options() const;

#ifdef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;
#else
    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;
#endif

    EndpointIPtr endpoint(const AcceptorIPtr&) const;

    using IceInternal::IPEndpointI::connectionId;

protected:

    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual void hashInit(Ice::Int&) const;
    virtual void initWithOptions(std::vector<std::string>&, bool);
    virtual void fillEndpointInfo(Ice::IPEndpointInfo*) const;
    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

    virtual IceInternal::ConnectorPtr createConnector(const IceInternal::Address&, const IceInternal::NetworkProxyPtr&) const;
    virtual IceInternal::IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const Ice::Int _timeout;
    const bool _compress;
};

class EndpointFactoryI : public IceInternal::EndpointFactory
{
public:

    EndpointFactoryI(const IceInternal::ProtocolInstancePtr&);

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const;

private:

    IceInternal::ProtocolInstancePtr _instance;
};

}

#endif
