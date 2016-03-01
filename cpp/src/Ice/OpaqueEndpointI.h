// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UNKNOWN_ENDPOINT_I_H
#define ICE_UNKNOWN_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

class OpaqueEndpointI : public EndpointI
{
public:

    OpaqueEndpointI(std::vector<std::string>&);
    OpaqueEndpointI(Ice::Short, BasicStream*);

    virtual void streamWrite(BasicStream*) const;
    virtual Ice::EndpointInfoPtr getInfo() const;
    virtual Ice::Short type() const;
    virtual const std::string& protocol() const;

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
    virtual Ice::Int hash() const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::LocalObject&) const;
    virtual bool operator<(const Ice::LocalObject&) const;


    using EndpointI::connectionId;

protected:

    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

private:

    //
    // All members are const, because endpoints are immutable.
    //
    Ice::Short _type;
    Ice::EncodingVersion _rawEncoding; // The encoding used for _rawBytes
    const std::vector<Ice::Byte> _rawBytes;
};

}

#endif
