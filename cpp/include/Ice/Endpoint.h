// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include "EndpointF.h"
#include "Ice/EndpointTypes.h"
#include "Ice/Version.h"

#include <string>
#include <vector>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Ice
{
    /// The user-level interface to an endpoint.
    /// \headerfile Ice/Ice.h
    class ICE_API Endpoint
    {
    public:
        /// Default constructor.
        Endpoint() = default;

        /// Destructor.
        virtual ~Endpoint();

        Endpoint(const Endpoint&) = delete;
        Endpoint& operator=(const Endpoint&) = delete;

        /// Operator equal to. Checks if this endpoint is equal to the @p rhs endpoint.
        /// @param rhs The endpoint to compare against.
        /// @return `true` if the endpoints are equal, `false` otherwise.
        virtual bool operator==(const Endpoint& rhs) const = 0;

        /// Operator less than. Checks if this endpoint is less than the @p rhs endpoint.
        /// @param rhs The endpoint to compare against.
        /// @return `true` if this endpoint is less than the @p rhs endpoint, `false` otherwise.
        virtual bool operator<(const Endpoint& rhs) const = 0;

        /// Return a string representation of the endpoint.
        /// @return The string representation of the endpoint.
        [[nodiscard]] virtual std::string toString() const noexcept = 0;

        /// Returns the endpoint information.
        /// @return The endpoint information class.
        [[nodiscard]] virtual EndpointInfoPtr getInfo() const noexcept = 0;
    };

    /// Base class providing access to the endpoint details.
    /// \headerfile Ice/Ice.h
    class ICE_API EndpointInfo
    {
    public:
        virtual ~EndpointInfo();
        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        /// Returns the type of the endpoint.
        /// @return The endpoint type.
        [[nodiscard]] virtual std::int16_t type() const noexcept;

        /// Returns true if this endpoint is a datagram endpoint.
        /// @return True for a datagram endpoint.
        [[nodiscard]] virtual bool datagram() const noexcept;

        /// @return True for a secure endpoint.
        [[nodiscard]] virtual bool secure() const noexcept;

        /// The information of the underlying endpoint or null if there's no underlying endpoint.
        const EndpointInfoPtr underlying;
        /// The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
        const int timeout;
        /// Specifies whether or not compression should be used if available when using this endpoint.
        const bool compress;

    protected:
        /// @private
        explicit EndpointInfo(EndpointInfoPtr underlyingInfo)
            : underlying(std::move(underlyingInfo)),
              timeout(underlying->timeout),
              compress(underlying->compress)
        {
        }

        /// @private
        EndpointInfo(int timeout, bool compress) : timeout(timeout), compress(compress) {}
    };

    /// Provides access to the address details of a IP endpoint.
    /// @see Endpoint
    /// \headerfile Ice/Ice.h
    class ICE_API IPEndpointInfo : public EndpointInfo
    {
    public:
        IPEndpointInfo(const IPEndpointInfo&) = delete;
        IPEndpointInfo& operator=(const IPEndpointInfo&) = delete;

        ~IPEndpointInfo() override;

        /// The host or address configured with the endpoint.
        const std::string host;
        /// The port number.
        const int port;
        /// The source IP address.
        const std::string sourceAddress;

    protected:
        /// @private
        IPEndpointInfo(int timeout, bool compress, std::string host, int port, std::string sourceAddress)
            : EndpointInfo{timeout, compress},
              host{std::move(host)},
              port{port},
              sourceAddress{std::move(sourceAddress)}
        {
        }
    };

    /// Provides access to a TCP endpoint information.
    /// @see Endpoint
    /// \headerfile Ice/Ice.h
    class ICE_API TCPEndpointInfo final : public IPEndpointInfo
    {
    public:
        ~TCPEndpointInfo() final;
        TCPEndpointInfo(const TCPEndpointInfo&) = delete;
        TCPEndpointInfo& operator=(const TCPEndpointInfo&) = delete;

        [[nodiscard]] std::int16_t type() const noexcept final { return _type; }
        [[nodiscard]] bool secure() const noexcept final { return _secure; }

        // internal constructor
        /// @private
        TCPEndpointInfo(
            int timeout,
            bool compress,
            std::string host,
            int port,
            std::string sourceAddress,
            std::int16_t type,
            bool secure)
            : IPEndpointInfo{timeout, compress, std::move(host), port, std::move(sourceAddress)},
              _type{type},
              _secure{secure}
        {
        }

    private:
        const std::int16_t _type;
        const bool _secure;
    };

    /// Provides access to an UDP endpoint information.
    /// @see Endpoint
    /// \headerfile Ice/Ice.h
    class ICE_API UDPEndpointInfo final : public IPEndpointInfo
    {
    public:
        ~UDPEndpointInfo() final;
        UDPEndpointInfo(const UDPEndpointInfo&) = delete;
        UDPEndpointInfo& operator=(const UDPEndpointInfo&) = delete;

        /// The multicast interface.
        const std::string mcastInterface;
        /// The multicast time-to-live (or hops).
        const int mcastTtl;

        [[nodiscard]] std::int16_t type() const noexcept final { return UDPEndpointType; }
        [[nodiscard]] bool datagram() const noexcept final { return true; }

        // internal constructor
        /// @private
        UDPEndpointInfo(
            bool compress,
            std::string host,
            int port,
            std::string sourceAddress,
            std::string mcastInterface,
            int mcastTtl)
            : IPEndpointInfo{-1, compress, std::move(host), port, std::move(sourceAddress)},
              mcastInterface{std::move(mcastInterface)},
              mcastTtl{mcastTtl}
        {
        }
    };

    /// Provides access to a WebSocket endpoint information.
    /// \headerfile Ice/Ice.h
    class ICE_API WSEndpointInfo final : public EndpointInfo
    {
    public:
        ~WSEndpointInfo() final;
        WSEndpointInfo(const WSEndpointInfo&) = delete;
        WSEndpointInfo& operator=(const WSEndpointInfo&) = delete;

        /// The URI configured with the endpoint.
        const std::string resource;

        // internal constructor
        /// @private
        WSEndpointInfo(EndpointInfoPtr underlying, std::string resource)
            : EndpointInfo{std::move(underlying)},
              resource{std::move(resource)}
        {
        }
    };

    /// Provides access to an IAP endpoint information.
    /// \headerfile Ice/Ice.h
    class IAPEndpointInfo final : public EndpointInfo
    {
    public:
        ~IAPEndpointInfo() final;
        IAPEndpointInfo(const IAPEndpointInfo&) = delete;
        IAPEndpointInfo& operator=(const IAPEndpointInfo&) = delete;

        [[nodiscard]] std::int16_t type() const noexcept final { return _type; }
        [[nodiscard]] bool secure() const noexcept final { return _secure; }

        /// The accessory manufacturer or empty to not match against
        /// a manufacturer.
        const std::string manufacturer;
        /// The accessory model number or empty to not match against
        /// a model number.
        const std::string modelNumber;
        /// The accessory name or empty to not match against
        /// the accessory name.
        const std::string name;
        /// The protocol supported by the accessory.
        const std::string protocol;

        // internal constructor
        /// @private
        IAPEndpointInfo(
            int timeout,
            bool compress,
            std::string manufacturer,
            std::string modelNumber,
            std::string name,
            std::string protocol,
            std::int16_t type,
            bool secure)
            : EndpointInfo{timeout, compress},
              manufacturer{std::move(manufacturer)},
              modelNumber{std::move(modelNumber)},
              name{std::move(name)},
              protocol{std::move(protocol)},
              _type{type},
              _secure{secure}
        {
        }

    private:
        const std::int16_t _type;
        const bool _secure;
    };

    /// Provides access to the details of an opaque endpoint.
    /// @see Endpoint
    /// \headerfile Ice/Ice.h
    class ICE_API OpaqueEndpointInfo final : public EndpointInfo
    {
    public:
        ~OpaqueEndpointInfo() final;
        OpaqueEndpointInfo(const OpaqueEndpointInfo&) = delete;
        OpaqueEndpointInfo& operator=(const OpaqueEndpointInfo&) = delete;

        [[nodiscard]] std::int16_t type() const noexcept final { return _type; }

        /// The encoding version of the opaque endpoint (to decode or encode the rawBytes).
        const Ice::EncodingVersion rawEncoding;
        /// The raw encoding of the opaque endpoint.
        const std::vector<std::byte> rawBytes;

        // internal constructor
        /// @private
        OpaqueEndpointInfo(std::int16_t type, Ice::EncodingVersion rawEncoding, std::vector<std::byte> rawBytes)
            : EndpointInfo{-1, false},
              rawEncoding{rawEncoding},
              rawBytes{std::move(rawBytes)},
              _type{type}
        {
        }

    private:
        std::int16_t _type;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
