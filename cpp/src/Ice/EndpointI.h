//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_I_H
#define ICE_ENDPOINT_I_H

#include "AcceptorF.h"
#include "ConnectorF.h"
#include "EndpointIF.h"
#include "Ice/Endpoint.h"
#include "Ice/EndpointSelectionType.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "TransceiverF.h"

#include <optional>

namespace Ice
{
    class OutputStream;
    class InputStream;
}

namespace IceInternal
{
    class ICE_API EndpointI : public Ice::Endpoint
    {
    public:
        //
        // Marshal the endpoint.
        //
        virtual void streamWrite(Ice::OutputStream*) const;
        virtual void streamWriteImpl(Ice::OutputStream*) const = 0;

        //
        // Return the endpoint type.
        //
        virtual std::int16_t type() const = 0;

        //
        // Return the protocol name
        //
        virtual const std::string& protocol() const = 0;

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        virtual std::int32_t timeout() const = 0;

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        virtual EndpointIPtr timeout(std::int32_t) const = 0;

        //
        // Returns the endpoint connection id.
        //
        virtual const std::string& connectionId() const = 0;

        //
        // Return a new endpoint with a different connection id.
        //
        virtual EndpointIPtr connectionId(const std::string&) const = 0;

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        virtual bool compress() const = 0;

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        virtual EndpointIPtr compress(bool) const = 0;

        //
        // Return true if the endpoint is datagram-based.
        //
        virtual bool datagram() const = 0;

        //
        // Return true if the endpoint is secure.
        //
        virtual bool secure() const = 0;

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        virtual TransceiverPtr transceiver() const = 0;

        //
        // Return connectors for this endpoint, or empty vector if no
        // connector is available. Implementation is responsible for
        // returning connectors sorted according to the endpoint selection
        // type.
        //
        virtual void connectorsAsync(
            Ice::EndpointSelectionType,
            std::function<void(std::vector<ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const = 0;

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        virtual AcceptorPtr
        acceptor(const std::string&, const std::optional<Ice::SSL::ServerAuthenticationOptions>&) const = 0;

        // Expand endpoint into separate endpoints for each IP address returned by the DNS resolver.
        // Used only for server endpoints.
        virtual std::vector<EndpointIPtr> expandHost() const = 0;

        // Returns true when the most underlying endpoint is an IP endpoint with a loopback or multicast address.
        virtual bool isLoopbackOrMulticast() const = 0;

        // Returns a new endpoint with the specified host (if not empty) and all local options cleared. May return
        // shared_from_this().
        virtual std::shared_ptr<EndpointI> toPublishedEndpoint(std::string publishedHost) const = 0;

        //
        // Check whether the endpoint is equivalent to another one.
        //
        virtual bool equivalent(const EndpointIPtr&) const = 0;

        virtual std::size_t hash() const noexcept = 0;

        //
        // Returns the stringified options
        //
        virtual std::string options() const = 0;

        virtual std::string toString() const noexcept;
        void initWithOptions(std::vector<std::string>&);

    protected:
        virtual bool checkOption(const std::string&, const std::string&, const std::string&);
    };

    template<typename T> class InfoI final : public T
    {
    public:
        InfoI(const EndpointIPtr& endpoint) : _endpoint(endpoint)
        {
            T::compress = _endpoint->compress();
            T::timeout = _endpoint->timeout();
        }

        ~InfoI() = default;

        std::int16_t type() const noexcept final { return _endpoint->type(); }

        bool datagram() const noexcept final { return _endpoint->datagram(); }

        bool secure() const noexcept final { return _endpoint->secure(); }

    private:
        const EndpointIPtr _endpoint;
    };
}

namespace std
{
    /// Specialization of std::hash for EndpointI.
    template<> struct hash<IceInternal::EndpointI>
    {
        std::size_t operator()(const IceInternal::EndpointI& endpoint) const noexcept { return endpoint.hash(); }
    };
}

#endif
