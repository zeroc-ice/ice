// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_ENDPOINT_I_H
#define ICE_BT_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>
#include <IceBT/Config.h>
#include <IceBT/EndpointInfo.h>
#include <IceBT/Engine.h>
#include <IceBT/InstanceF.h>
#include <IceUtil/Thread.h>

namespace IceBT
{

class EndpointI : public IceInternal::EndpointI
#ifdef ICE_CPP11_MAPPING
                , public std::enable_shared_from_this<EndpointI>
#endif
{
public:

    EndpointI(const InstancePtr&, const std::string&, const std::string&, const std::string&, Ice::Int,
              Ice::Int, const std::string&, bool);
    EndpointI(const InstancePtr&);
    EndpointI(const InstancePtr&, Ice::InputStream*);

    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;
    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual const std::string& connectionId() const;
    virtual IceInternal::EndpointIPtr connectionId(const std::string&) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;
    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expand() const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;

#ifdef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;
#else
    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;
#endif

    virtual Ice::Int hash() const;

    virtual std::string options() const;

    Ice::EndpointInfoPtr getInfo() const;

    void initWithOptions(std::vector<std::string>&, bool);

    EndpointIPtr endpoint(const AcceptorIPtr&) const;

private:

    void hashInit();
    bool checkOption(const std::string&, const std::string&, const std::string&);

    const InstancePtr _instance;
    const std::string _addr;
    const std::string _uuid;
    const std::string _name;
    const Ice::Int _channel;
    const Ice::Int _timeout;
    const std::string _connectionId;
    const bool _compress;
    const Ice::Int _hashValue;
};

class EndpointInfoI : public EndpointInfo
{
public:

    EndpointInfoI(const EndpointIPtr&);
    virtual ~EndpointInfoI();

    virtual Ice::Short type() const;
    virtual bool datagram() const;
    virtual bool secure() const;

private:

    const EndpointIPtr _endpoint;
};

class EndpointFactoryI : public IceInternal::EndpointFactory
{
public:

    virtual ~EndpointFactoryI();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&,
                                                  const IceInternal::EndpointFactoryPtr&) const;

private:

    EndpointFactoryI(const InstancePtr&);
    friend class PluginI;

    InstancePtr _instance;
};

}

#endif
