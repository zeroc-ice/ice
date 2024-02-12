//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

class EndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
{
public:

    EndpointI(const InstancePtr&, const IceInternal::EndpointIPtr&);

    void streamWriteImpl(Ice::OutputStream*) const final;

    Ice::EndpointInfoPtr getInfo() const noexcept final;
    Ice::Short type() const final;
    const std::string& protocol() const final;

    Ice::Int timeout() const final;
    IceInternal::EndpointIPtr timeout(Ice::Int) const final;
    const std::string& connectionId() const final;
    IceInternal::EndpointIPtr connectionId(const ::std::string&) const final;
    bool compress() const final;
    IceInternal::EndpointIPtr compress(bool) const final;
    bool datagram() const final;
    bool secure() const final;

    IceInternal::TransceiverPtr transceiver() const final;
    void connectorsAsync(
        Ice::EndpointSelectionType,
        std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
        std::function<void(std::exception_ptr)>) const final;
    IceInternal::AcceptorPtr acceptor(const std::string&) const final;
    std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const final;
    std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const final;
    bool equivalent(const IceInternal::EndpointIPtr&) const final;
    ::Ice::Int hash() const final;
    std::string options() const final;

    EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

    bool operator==(const Ice::Endpoint&) const final;
    bool operator<(const Ice::Endpoint&) const final;

protected:

    bool checkOption(const std::string&, const std::string&, const std::string&) final;

private:

    //
    // All members are const, because endpoints are immutable.
    //
    const InstancePtr _instance;
    const IceInternal::EndpointIPtr _delegate;
};

class EndpointFactoryI final : public IceInternal::EndpointFactoryWithUnderlying
{
public:

    EndpointFactoryI(const InstancePtr&, Ice::Short);

    void destroy() final;

    IceInternal::EndpointFactoryPtr
    cloneWithUnderlying(const IceInternal::ProtocolInstancePtr&, Ice::Short) const final;

protected:

    IceInternal::EndpointIPtr
    createWithUnderlying(const IceInternal::EndpointIPtr&, std::vector<std::string>&, bool) const final;
    IceInternal::EndpointIPtr
    readWithUnderlying(const IceInternal::EndpointIPtr&, Ice::InputStream*) const final;

private:

    InstancePtr _sslInstance;
};

}

#endif
