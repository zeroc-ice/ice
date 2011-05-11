// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ENDPOINT_I_H
#define TEST_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Test.h>
#include <Configuration.h>

class EndpointI : public IceInternal::EndpointI
{
public:

    static Ice::Short TYPE_BASE;

    // From EndpointI
    virtual void streamWrite(IceInternal::BasicStream*) const;
    virtual Ice::Short type() const;
    virtual IceInternal::EndpointIPtr timeout(Ice::Int) const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual IceInternal::TransceiverPtr transceiver(IceInternal::EndpointIPtr&) const;
    virtual std::vector<IceInternal::ConnectorPtr> connectors() const;
    virtual void connectors_async(const IceInternal::EndpointI_connectorsPtr&) const;
    virtual IceInternal::AcceptorPtr acceptor(IceInternal::EndpointIPtr&, const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expand() const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;

    // From TestEndpoint
    virtual std::string toString() const;
    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::Int timeout() const;
    virtual bool compress() const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;

protected:

    virtual Ice::Int hashInit() const;
#if !defined(_MSC_VER) || _MSC_VER > 1300
    using IceInternal::EndpointI::connectors;
#endif
    
private:

    EndpointI(const IceInternal::EndpointIPtr&);
    friend class EndpointFactory;

    const IceInternal::EndpointIPtr _endpoint;
    const ConfigurationPtr _configuration;
};

#endif
