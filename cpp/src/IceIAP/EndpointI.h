//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_ENDPOINT_I_H
#define ICE_IAP_ENDPOINT_I_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceObjC
{

class iAPEndpointI;
typedef ::std::shared_ptr<iAPEndpointI> iAPEndpointIPtr;

class iAPEndpointI final : public IceInternal::EndpointI, public std::enable_shared_from_this<iAPEndpointI>
{
public:

    iAPEndpointI(const IceInternal::ProtocolInstancePtr&, const std::string&, const std::string&, const std::string&,
                 const std::string&, Ice::Int, const std::string&, bool);
    iAPEndpointI(const IceInternal::ProtocolInstancePtr&);
    iAPEndpointI(const IceInternal::ProtocolInstancePtr&, Ice::InputStream*);

    void streamWriteImpl(Ice::OutputStream*) const final;

    Ice::EndpointInfoPtr getInfo() const noexcept final;
    Ice::Short type() const final;
    const std::string& protocol() const final;
    bool datagram() const final;
    bool secure() const final;

    Ice::Int timeout() const final;
    IceInternal::EndpointIPtr timeout(Ice::Int) const final;
    const std::string& connectionId() const final;
    IceInternal::EndpointIPtr connectionId(const std::string&) const final;
    bool compress() const final;
    IceInternal::EndpointIPtr compress(bool) const final;

    IceInternal::TransceiverPtr transceiver() const final;
    void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const final;
    IceInternal::AcceptorPtr acceptor(const std::string&) const final;
    std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const final;
    std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const final;
    bool equivalent(const IceInternal::EndpointIPtr&) const final;

    bool operator==(const Ice::Endpoint&) const final;
    bool operator<(const Ice::Endpoint&) const final;

    std::string options() const final;
    ::Ice::Int hash() const final;

private:

    bool checkOption(const std::string&, const std::string&, const std::string&) final;

    //
    // All members are const, because endpoints are immutable.
    //
    const IceInternal::ProtocolInstancePtr _instance;
    const std::string _manufacturer;
    const std::string _modelNumber;
    const std::string _name;
    const std::string _protocol;
    const Ice::Int _timeout;
    const std::string _connectionId;
    const bool _compress;
};

class iAPEndpointFactory final : public IceInternal::EndpointFactory
{
public:

    iAPEndpointFactory(const IceInternal::ProtocolInstancePtr&);

    ~iAPEndpointFactory();

    Ice::Short type() const final;
    std::string protocol() const final;
    IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const final;
    IceInternal::EndpointIPtr read(Ice::InputStream*) const final;
    void destroy() final;

    IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const final;

private:

    IceInternal::ProtocolInstancePtr _instance;
};

}

#endif
