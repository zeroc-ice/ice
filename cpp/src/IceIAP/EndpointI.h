// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_IAP_ENDPOINT_I_H
#define ICE_IAP_ENDPOINT_I_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceObjC
{

class iAPEndpointI;
#ifdef ICE_CPP11_MAPPING // C++11 mapping
typedef ::std::shared_ptr<iAPEndpointI> iAPEndpointIPtr;
#else
typedef IceUtil::Handle<iAPEndpointI> iAPEndpointIPtr;
#endif

class iAPEndpointI : public IceInternal::EndpointI
#ifdef ICE_CPP11_MAPPING
                   , public std::enable_shared_from_this<iAPEndpointI>
#endif
{
public:

    iAPEndpointI(const IceInternal::ProtocolInstancePtr&, const std::string&, const std::string&, const std::string&,
                 const std::string&, Ice::Int, const std::string&, bool);
    iAPEndpointI(const IceInternal::ProtocolInstancePtr&);
    iAPEndpointI(const IceInternal::ProtocolInstancePtr&, Ice::InputStream*);

    virtual void streamWriteImpl(Ice::OutputStream*) const;

    virtual Ice::EndpointInfoPtr getInfo() const ICE_NOEXCEPT;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual Ice::Int timeout() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual const std::string& connectionId() const;
    virtual IceInternal::EndpointIPtr connectionId(const std::string&) const;
    virtual bool compress() const;
    virtual IceInternal::EndpointIPtr compress(bool) const;

    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;

#ifdef ICE_CPP11_MAPPING
    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;
#else
    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;
#endif

    virtual std::string options() const;
    virtual ::Ice::Int hash() const;

private:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

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

class iAPEndpointFactory : public IceInternal::EndpointFactory
{
public:

    iAPEndpointFactory(const IceInternal::ProtocolInstancePtr&);

    virtual ~iAPEndpointFactory();

    virtual Ice::Short type() const;
    virtual std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const;

private:

    IceInternal::ProtocolInstancePtr _instance;
};

}

#endif
