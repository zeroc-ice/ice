//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAM_ENDPOINT_I_H
#define ICE_STREAM_ENDPOINT_I_H

#include <Ice/Config.h>

#if TARGET_OS_IPHONE != 0

#include <Ice/ProtocolInstance.h>
#include <Ice/IPEndpointI.h>
#include <Ice/WSEndpoint.h>
#include <Ice/EndpointFactory.h>
#include <Ice/InstanceF.h>
#include <Ice/UniqueRef.h>

#include <CoreFoundation/CFDictionary.h>
#include <CFNetwork/CFNetwork.h>

namespace Ice
{

class OutputStream;
class InputStream;

}

namespace IceObjC
{

class Instance : public IceInternal::ProtocolInstance
{
public:

    Instance(const Ice::CommunicatorPtr&, Ice::Short, const std::string&, bool);
    Instance(const Instance&, const IceInternal::ProtocolInstancePtr&);
    ~Instance() = default;

    const std::string& proxyHost() const
    {
        return _proxyHost;
    }

    int proxyPort() const
    {
        return _proxyPort;
    }

    void setupStreams(CFReadStreamRef, CFWriteStreamRef, bool, const std::string&) const;

private:

    const Ice::CommunicatorPtr _communicator;
    IceInternal::UniqueRef<CFMutableDictionaryRef> _proxySettings;
    std::string _proxyHost;
    int _proxyPort;
};

using InstancePtr = std::shared_ptr<Instance>;

class StreamAcceptor;
using StreamAcceptorPtr = std::shared_ptr<StreamAcceptor>;

class StreamEndpointI;
using StreamEndpointIPtr = std::shared_ptr<StreamEndpointI>;

class StreamEndpointI : public IceInternal::IPEndpointI
{
public:

    StreamEndpointI(const InstancePtr&, const std::string&, Ice::Int, const IceInternal::Address&, Ice::Int,
                    const std::string&, bool);
    StreamEndpointI(const InstancePtr&);
    StreamEndpointI(const InstancePtr&, Ice::InputStream*);

    virtual Ice::EndpointInfoPtr getInfo() const noexcept;

    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::string options() const;

    std::shared_ptr<StreamEndpointI> shared_from_this()
    {
        return std::static_pointer_cast<StreamEndpointI>(IceInternal::IPEndpointI::shared_from_this());
    }

    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;

    StreamEndpointIPtr endpoint(const StreamAcceptorPtr&) const;

    using IPEndpointI::connectionId;

protected:

    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual void hashInit(Ice::Int&) const;
    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

    virtual IceInternal::ConnectorPtr createConnector(const IceInternal::Address&,
                                                      const IceInternal::NetworkProxyPtr&) const;
    virtual IceInternal::IPEndpointIPtr createEndpoint(const std::string&, int, const std::string&) const;

private:

    const InstancePtr _streamInstance;

    //
    // All members are const, because endpoints are immutable.
    //
    const Ice::Int _timeout;
    const bool _compress;
};

class StreamEndpointFactory : public IceInternal::EndpointFactory
{
public:

    StreamEndpointFactory(const InstancePtr&);
    ~StreamEndpointFactory() = default;

    Ice::Short type() const final;
    std::string protocol() const final;
    IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
    IceInternal::EndpointIPtr read(Ice::InputStream*) const final;
    void destroy() final;

    IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

private:

    InstancePtr _instance;
};

}

#endif

#endif
