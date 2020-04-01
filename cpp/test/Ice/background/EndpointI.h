//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ENDPOINT_I_H
#define TEST_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Test.h>
#include <Configuration.h>

class EndpointI;
ICE_DEFINE_PTR(EndpointIPtr, EndpointI);

class EndpointI : public IceInternal::EndpointI
                , public std::enable_shared_from_this<EndpointI>
{
public:

    static Ice::Short TYPE_BASE;

    EndpointI(const IceInternal::EndpointIPtr&);

    // From EndpointI
    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual void connectors_async(Ice::EndpointSelectionType, const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;

    // From TestEndpoint
    virtual std::string toString() const noexcept;
    virtual Ice::EndpointInfoPtr getInfo() const noexcept;
    virtual Ice::Int timeout() const;
    virtual const std::string& connectionId() const;
    virtual bool compress() const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;

    virtual int hash() const;
    virtual std::string options() const;

    IceInternal::EndpointIPtr delegate() const;
    EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

    using IceInternal::EndpointI::connectionId;

private:

    friend class EndpointFactory;

    const IceInternal::EndpointIPtr _endpoint;
    const ConfigurationPtr _configuration;
};

#endif
