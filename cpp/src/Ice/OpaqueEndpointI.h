//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UNKNOWN_ENDPOINT_I_H
#define ICE_UNKNOWN_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Ice/EndpointFactory.h>

namespace IceInternal
{

class OpaqueEndpointI : public EndpointI, public std::enable_shared_from_this<OpaqueEndpointI>
{
public:

    OpaqueEndpointI(std::vector<std::string>&);
    OpaqueEndpointI(std::int16_t, Ice::InputStream*);

    virtual void streamWrite(Ice::OutputStream*) const;
    virtual Ice::EndpointInfoPtr getInfo() const noexcept;
    virtual std::int16_t type() const;
    virtual const std::string& protocol() const;

    virtual std::int32_t timeout() const;
    virtual EndpointIPtr timeout(std::int32_t) const;
    virtual const std::string& connectionId() const;
    virtual EndpointIPtr connectionId(const ::std::string&) const;
    virtual bool compress() const;
    virtual EndpointIPtr compress(bool) const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual TransceiverPtr transceiver() const;
    virtual void connectorsAsync(
        Ice::EndpointSelectionType,
        std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
        std::function<void(std::exception_ptr)>) const;
    virtual AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<EndpointIPtr> expandHost(EndpointIPtr&) const;
    virtual bool equivalent(const EndpointIPtr&) const;
    virtual std::int32_t hash() const;
    virtual std::string options() const;

    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;

    using EndpointI::connectionId;

protected:

    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual bool checkOption(const std::string&, const std::string&, const std::string&);

private:

    //
    // All members are const, because endpoints are immutable.
    //
    std::int16_t _type;
    Ice::EncodingVersion _rawEncoding; // The encoding used for _rawBytes
    const std::vector<std::uint8_t> _rawBytes;
};

}

#endif
