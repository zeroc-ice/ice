// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_WS_ENDPOINT_I_H
#define ICE_WS_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/IPEndpointIF.h>
#include <Ice/EndpointFactory.h>
#include <Ice/InstanceF.h>
#include <Ice/Endpoint.h>
#include <Ice/ProtocolInstance.h>

namespace IceInternal
{

class WSEndpoint : public EndpointI
#ifdef ICE_CPP11_MAPPING
                 , public std::enable_shared_from_this<WSEndpoint>
#endif
{
public:

    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, const std::string&);
    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, std::vector<std::string>&);
    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, Ice::InputStream*);

    virtual void streamWriteImpl(Ice::OutputStream*) const;

    virtual Ice::EndpointInfoPtr getInfo() const ICE_NOEXCEPT;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;

    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual const std::string& connectionId() const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual TransceiverPtr transceiver() const;
    virtual void connectors_async(Ice::EndpointSelectionType, const EndpointI_connectorsPtr&) const;
    virtual AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<EndpointIPtr> expandHost(EndpointIPtr&) const;
    virtual bool equivalent(const EndpointIPtr&) const;
    virtual ::Ice::Int hash() const;
    virtual std::string options() const;

    WSEndpointPtr endpoint(const EndpointIPtr&) const;

#ifdef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;
#else
    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;
#endif

protected:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const ProtocolInstancePtr _instance;
    const EndpointIPtr _delegate;
    const std::string _resource;
};

class ICE_API WSEndpointFactory : public EndpointFactoryWithUnderlying
{
public:

    WSEndpointFactory(const ProtocolInstancePtr&, Ice::Short);

    virtual EndpointFactoryPtr cloneWithUnderlying(const ProtocolInstancePtr&, Ice::Short) const;

protected:

    virtual EndpointIPtr createWithUnderlying(const EndpointIPtr&, std::vector<std::string>&, bool) const;
    virtual EndpointIPtr readWithUnderlying(const EndpointIPtr&, Ice::InputStream*) const;
};

}

#endif
