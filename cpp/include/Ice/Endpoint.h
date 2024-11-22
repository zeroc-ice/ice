//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include "EndpointF.h"
#include "Ice/Version.h"

#include <string>
#include <vector>

namespace Ice
{
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
     * Base class providing access to the endpoint details.
     * \headerfile Ice/Ice.h
     */
    class ICE_API EndpointInfo
    {
    public:
        EndpointInfo() = default;
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
     * Provides access to the address details of a IP endpoint.
     * @see Endpoint
     * \headerfile Ice/Ice.h
     */
    class ICE_API IPEndpointInfo : public EndpointInfo
    {
    public:
        IPEndpointInfo() = default;
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

#endif
