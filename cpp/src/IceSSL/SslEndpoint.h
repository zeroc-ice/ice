// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ENDPOINT_H
#define ICE_SSL_ENDPOINT_H

#include <Ice/Endpoint.h>
#include <Ice/EndpointFactory.h>
#include <IceSSL/OpenSSLPluginIF.h>

namespace IceSSL
{

const Ice::Short SslEndpointType = 2;

class SslEndpoint : public IceInternal::Endpoint
{
public:

    SslEndpoint(const IceSSL::OpenSSLPluginIPtr&, const std::string&, Ice::Int, Ice::Int, bool);
    SslEndpoint(const IceSSL::OpenSSLPluginIPtr&, const std::string&);
    SslEndpoint(const IceSSL::OpenSSLPluginIPtr&, IceInternal::BasicStream*);

    virtual void streamWrite(IceInternal::BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::Short type() const;
    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointPtr timeout(Ice::Int) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual bool unknown() const;
    virtual IceInternal::TransceiverPtr clientTransceiver() const;
    virtual IceInternal::TransceiverPtr serverTransceiver(IceInternal::EndpointPtr&) const;
    virtual IceInternal::ConnectorPtr connector() const;
    virtual IceInternal::AcceptorPtr acceptor(IceInternal::EndpointPtr&) const;
    virtual bool equivalent(const IceInternal::TransceiverPtr&) const;
    virtual bool equivalent(const IceInternal::AcceptorPtr&) const;

    virtual bool operator==(const IceInternal::Endpoint&) const;
    virtual bool operator!=(const IceInternal::Endpoint&) const;
    virtual bool operator<(const IceInternal::Endpoint&) const;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const IceSSL::OpenSSLPluginIPtr _plugin;
    const std::string _host;
    const Ice::Int _port;
    const Ice::Int _timeout;
    const bool _compress;
};

class SslEndpointFactory : public IceInternal::EndpointFactory
{
public:

    SslEndpointFactory(const IceSSL::OpenSSLPluginIPtr&);
    virtual ~SslEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual IceInternal::EndpointPtr create(const std::string&) const;
    virtual IceInternal::EndpointPtr read(IceInternal::BasicStream*) const;
    virtual void destroy();

private:

    IceSSL::OpenSSLPluginIPtr _plugin;
};

}

#endif
