// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_ENDPOINT_I_H
#define ICESSL_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/IPEndpointI.h>
#include <Ice/EndpointFactory.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/EndpointInfo.h>
#include <IceSSL/SSLEngineF.h>
#include <Ice/Network.h>

namespace IceSSL
{

class EndpointI : public IceInternal::EndpointI
#ifdef ICE_CPP11_MAPPING
                , public std::enable_shared_from_this<EndpointI>
#endif
{
public:

    EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&);

    virtual void streamWriteImpl(Ice::OutputStream*) const;

    virtual Ice::EndpointInfoPtr getInfo() const ICE_NOEXCEPT;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;

    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual const std::string& connectionId() const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;
    virtual ::Ice::Int hash() const;
    virtual std::string options() const;

    EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

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
    const InstancePtr _instance;
    const IceInternal::EndpointIPtr _delegate;
};

class EndpointFactoryI : public IceInternal::EndpointFactoryWithUnderlying
{
public:

    EndpointFactoryI(const InstancePtr&, Ice::Short);

    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr
    cloneWithUnderlying(const IceInternal::ProtocolInstancePtr&, Ice::Short) const;

protected:

    virtual IceInternal::EndpointIPtr
    createWithUnderlying(const IceInternal::EndpointIPtr&, std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr
    readWithUnderlying(const IceInternal::EndpointIPtr&, Ice::InputStream*) const;

private:

    InstancePtr _sslInstance;
};

}

#endif
