// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
#include <IceWS/InstanceF.h>
#include <IceWS/EndpointInfo.h>

namespace IceWS
{

class EndpointI : public IceInternal::EndpointI
{
public:

    EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&, const std::string&);
    EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&, std::vector<std::string>&);
    EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&, IceInternal::BasicStream*);

    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;
    virtual void streamWrite(IceInternal::BasicStream*) const;

    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual const std::string& connectionId() const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual IceInternal::TransceiverPtr transceiver(IceInternal::EndpointIPtr&) const;
    virtual std::vector<IceInternal::ConnectorPtr> connectors(Ice::EndpointSelectionType) const;
    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(IceInternal::EndpointIPtr&, const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expand() const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;
    virtual ::Ice::Int hash() const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

protected:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const IceInternal::IPEndpointIPtr _delegate;
    const std::string _resource;
};

class EndpointFactoryI : public IceInternal::EndpointFactory
{
public:

    virtual ~EndpointFactoryI();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr read(IceInternal::BasicStream*) const;
    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const;

private:

    EndpointFactoryI(const InstancePtr&, const IceInternal::EndpointFactoryPtr&);
    friend class PluginI;

    InstancePtr _instance;
    const IceInternal::EndpointFactoryPtr _delegate;
};

}

#endif
