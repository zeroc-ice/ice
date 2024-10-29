//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include "EndpointF.h"
#include "Ice/Version.h"

#include <string>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Ice
{
    /**
     * Base class providing access to the endpoint details.
     * \headerfile Ice/Ice.h
     */
    class ICE_API EndpointInfo
    {
    public:
        EndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         */
        EndpointInfo(EndpointInfoPtr underlying, int timeout, bool compress)
            : underlying(std::move(underlying)),
              timeout(timeout),
              compress(compress)
        {
        }

        virtual ~EndpointInfo();

        EndpointInfo(const EndpointInfo&) = delete;
        EndpointInfo& operator=(const EndpointInfo&) = delete;

        /**
         * Returns the type of the endpoint.
         * @return The endpoint type.
         */
        virtual short type() const noexcept = 0;

        /**
         * Returns true if this endpoint is a datagram endpoint.
         * @return True for a datagram endpoint.
         */
        virtual bool datagram() const noexcept = 0;

        /**
         * @return True for a secure endpoint.
         */
        virtual bool secure() const noexcept = 0;

        /**
         * The information of the underlying endpoint or null if there's no underlying endpoint.
         */
        EndpointInfoPtr underlying;
        /**
         * The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
         */
        int timeout;
        /**
         * Specifies whether or not compression should be used if available when using this endpoint.
         */
        bool compress;
    };

    /**
     * The user-level interface to an endpoint.
     * \headerfile Ice/Ice.h
     */
    class ICE_API Endpoint
    {
    public:
        Endpoint() = default;
        virtual ~Endpoint();

        Endpoint(const Endpoint&) = delete;
        Endpoint& operator=(const Endpoint&) = delete;

        virtual bool operator==(const Endpoint&) const = 0;
        virtual bool operator<(const Endpoint&) const = 0;

        /**
         * Return a string representation of the endpoint.
         * @return The string representation of the endpoint.
         */
        virtual std::string toString() const noexcept = 0;

        /**
         * Returns the endpoint information.
         * @return The endpoint information class.
         */
        virtual EndpointInfoPtr getInfo() const noexcept = 0;
    };

    /**
     * Provides access to the address details of a IP endpoint.
     * @see Endpoint
     * \headerfile Ice/Ice.h
     */
    class ICE_API IPEndpointInfo : public EndpointInfo
    {
    public:
        IPEndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param host The host or address configured with the endpoint.
         * @param port The port number.
         * @param sourceAddress The source IP address.
         */
        IPEndpointInfo(
            EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            std::string host,
            int port,
            std::string sourceAddress)
            : EndpointInfo(std::move(underlying), timeout, compress),
              host(std::move(host)),
              port(port),
              sourceAddress(std::move(sourceAddress))
        {
        }

        ~IPEndpointInfo() override;

        IPEndpointInfo(const IPEndpointInfo&) = delete;
        IPEndpointInfo& operator=(const IPEndpointInfo&) = delete;

        /**
         * The host or address configured with the endpoint.
         */
        std::string host;
        /**
         * The port number.
         */
        int port;
        /**
         * The source IP address.
         */
        std::string sourceAddress;
    };

    /**
     * Provides access to a TCP endpoint information.
     * @see Endpoint
     * \headerfile Ice/Ice.h
     */
    class ICE_API TCPEndpointInfo : public IPEndpointInfo
    {
    public:
        TCPEndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param host The host or address configured with the endpoint.
         * @param port The port number.
         * @param sourceAddress The source IP address.
         */
        TCPEndpointInfo(
            EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            std::string host,
            int port,
            std::string sourceAddress)
            : IPEndpointInfo(std::move(underlying), timeout, compress, std::move(host), port, std::move(sourceAddress))
        {
        }

        ~TCPEndpointInfo() override;

        TCPEndpointInfo(const TCPEndpointInfo&) = delete;
        TCPEndpointInfo& operator=(const TCPEndpointInfo&) = delete;
    };

    /**
     * Provides access to an UDP endpoint information.
     * @see Endpoint
     * \headerfile Ice/Ice.h
     */
    class ICE_API UDPEndpointInfo : public IPEndpointInfo
    {
    public:
        UDPEndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param host The host or address configured with the endpoint.
         * @param port The port number.
         * @param sourceAddress The source IP address.
         * @param mcastInterface The multicast interface.
         * @param mcastTtl The multicast time-to-live (or hops).
         */
        UDPEndpointInfo(
            EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            std::string host,
            int port,
            std::string sourceAddress,
            std::string mcastInterface,
            int mcastTtl)
            : IPEndpointInfo(std::move(underlying), timeout, compress, std::move(host), port, std::move(sourceAddress)),
              mcastInterface(std::move(mcastInterface)),
              mcastTtl(mcastTtl)
        {
        }

        ~UDPEndpointInfo() override;

        UDPEndpointInfo(const UDPEndpointInfo&) = delete;
        UDPEndpointInfo& operator=(const UDPEndpointInfo&) = delete;

        /**
         * The multicast interface.
         */
        std::string mcastInterface;
        /**
         * The multicast time-to-live (or hops).
         */
        int mcastTtl;
    };

    /**
     * Provides access to a WebSocket endpoint information.
     * \headerfile Ice/Ice.h
     */
    class ICE_API WSEndpointInfo : public EndpointInfo
    {
    public:
        WSEndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param resource The URI configured with the endpoint.
         */
        WSEndpointInfo(EndpointInfoPtr underlying, int timeout, bool compress, std::string resource)
            : EndpointInfo(std::move(underlying), timeout, compress),
              resource(std::move(resource))
        {
        }

        ~WSEndpointInfo() override;

        WSEndpointInfo(const WSEndpointInfo&) = delete;
        WSEndpointInfo& operator=(const WSEndpointInfo&) = delete;

        /**
         * The URI configured with the endpoint.
         */
        std::string resource;
    };

    /**
     * Provides access to the details of an opaque endpoint.
     * @see Endpoint
     * \headerfile Ice/Ice.h
     */
    class ICE_API OpaqueEndpointInfo : public EndpointInfo
    {
    public:
        OpaqueEndpointInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param rawEncoding The encoding version of the opaque endpoint (to decode or encode the rawBytes).
         * @param rawBytes The raw encoding of the opaque endpoint.
         */
        OpaqueEndpointInfo(
            EndpointInfoPtr underlying,
            int timeout,
            bool compress,
            EncodingVersion rawEncoding,
            std::vector<std::byte> rawBytes)
            : EndpointInfo(std::move(underlying), timeout, compress),
              rawEncoding(std::move(rawEncoding)),
              rawBytes(std::move(rawBytes))
        {
        }

        ~OpaqueEndpointInfo() override;

        OpaqueEndpointInfo(const OpaqueEndpointInfo&) = delete;
        OpaqueEndpointInfo& operator=(const OpaqueEndpointInfo&) = delete;

        /**
         * The encoding version of the opaque endpoint (to decode or encode the rawBytes).
         */
        Ice::EncodingVersion rawEncoding;
        /**
         * The raw encoding of the opaque endpoint.
         */
        std::vector<std::byte> rawBytes;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
