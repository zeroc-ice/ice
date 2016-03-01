// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

//
// Delegate interface implemented by TcpEndpoint or IceSSL::Endpoint or any endpoint that WS can
// delegate to.
//
class ICE_API WSEndpointDelegate : virtual public IceUtil::Shared
{
public:

    virtual Ice::EndpointInfoPtr getWSInfo(const std::string&) const = 0;
};

class WSEndpoint : public EndpointI
{
public:

    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, const std::string&);
    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, std::vector<std::string>&);
    WSEndpoint(const ProtocolInstancePtr&, const EndpointIPtr&, BasicStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;
    virtual void streamWrite(BasicStream*) const;

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

    virtual std::vector<EndpointIPtr> expand() const;
    virtual bool equivalent(const EndpointIPtr&) const;
    virtual ::Ice::Int hash() const;
    virtual std::string options() const;

    EndpointIPtr delegate() const;
    WSEndpointPtr endpoint(const EndpointIPtr&) const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

protected:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const ProtocolInstancePtr _instance;
    const IPEndpointIPtr _delegate;
    const std::string _resource;
};

class ICE_API WSEndpointFactory : public EndpointFactory
{
public:

    WSEndpointFactory(const ProtocolInstancePtr&, const EndpointFactoryPtr&);
    virtual ~WSEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual EndpointIPtr read(BasicStream*) const;
    virtual void destroy();

    virtual EndpointFactoryPtr clone(const ProtocolInstancePtr&) const;

private:

    ProtocolInstancePtr _instance;
    const EndpointFactoryPtr _delegate;
};

}

#endif
