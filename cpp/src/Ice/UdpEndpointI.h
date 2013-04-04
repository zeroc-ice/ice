// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_ENDPOINT_I_H
#define ICE_UDP_ENDPOINT_I_H

#include <IceUtil/Config.h>
#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>
#include <Ice/Network.h>

namespace IceInternal
{

class UdpEndpointI : public EndpointI
{
public:

    UdpEndpointI(const InstancePtr&, const std::string&, Ice::Int, const std::string&, Ice::Int, bool, 
                 const std::string&, bool);
    UdpEndpointI(const InstancePtr&, const std::string&, bool);
    UdpEndpointI(BasicStream*);

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual Ice::Int timeout() const;
    virtual EndpointIPtr timeout(Ice::Int) const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual TransceiverPtr transceiver(EndpointIPtr&) const;
    virtual std::vector<ConnectorPtr> connectors(Ice::EndpointSelectionType) const;
    virtual void connectors_async(Ice::EndpointSelectionType, const EndpointI_connectorsPtr&) const;
    virtual AcceptorPtr acceptor(EndpointIPtr&, const std::string&) const;
    virtual std::vector<EndpointIPtr> expand() const;
    virtual bool equivalent(const EndpointIPtr&) const;
    virtual std::vector<ConnectorPtr> connectors(const std::vector<Address>&, const NetworkProxyPtr&) const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

#ifdef __SUNPRO_CC
    using EndpointI::connectionId;
#endif
    
private:

    virtual ::Ice::Int hashInit() const;

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const std::string _mcastInterface;
    const Ice::Int _mcastTtl;
    const bool _connect;
    const bool _compress;
};

class UdpEndpointFactory : public EndpointFactory
{
public:

    virtual ~UdpEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(const std::string&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

private:

    UdpEndpointFactory(const InstancePtr&);
    friend class Instance;

    InstancePtr _instance;
};

}

#endif
